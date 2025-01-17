#include <fmt/core.h>

#include <functional>
#include <string>
#include <string_view>

#include "core.h"
#include "env.h"
#include "linenoise.h"
#include "printer.h"
#include "reader.h"

bool IsMacroCall(const std::shared_ptr<MalType>& ast,
                 const std::shared_ptr<Env>& env) {
    if (auto l = std::dynamic_pointer_cast<List>(ast)) {
        if ((*l)->empty()) return false;
        if (auto s = std::dynamic_pointer_cast<Symbol>((**l)[0])) {
            try {
                auto val = env->Get(**s);
                if (auto func = std::dynamic_pointer_cast<UserFunc>(val)) {
                    return func->is_macro();
                }
            } catch (...) {
                return false;
            }
        }
    }
    return false;
}

std::shared_ptr<MalType> MacroExpand(std::shared_ptr<MalType> ast,
                                     const std::shared_ptr<Env>& env) {
    while (IsMacroCall(ast, env)) {
        auto list = std::dynamic_pointer_cast<List>(ast);
        auto symbol = std::dynamic_pointer_cast<Symbol>((**list)[0]);
        auto func = std::dynamic_pointer_cast<UserFunc>(env->Get(**symbol));
        ast = (*func)(std::span{(*list)->begin() + 1, (*list)->end()});
    }
    return ast;
}

std::shared_ptr<MalType> Quasiquote(std::shared_ptr<MalType> ast) {
    if (auto seq = std::dynamic_pointer_cast<Sequence>(ast)) {
        if (!(*seq)->empty()) {
            if (auto l = std::dynamic_pointer_cast<List>(seq)) {
                if (auto s = std::dynamic_pointer_cast<Symbol>((**l)[0])) {
                    if (**s == "unquote") return (*l)->at(1);
                }
            }
        }
        auto list = std::make_shared<List>();
        for (auto it = (*seq)->rbegin(); it != (*seq)->rend(); it++) {
            if (auto subl = std::dynamic_pointer_cast<List>(*it)) {
                if (!(*subl)->empty()) {
                    if (auto s =
                            std::dynamic_pointer_cast<Symbol>((**subl)[0])) {
                        if (**s == "splice-unquote") {
                            auto new_list = std::make_shared<List>();
                            (*new_list)->assign(
                                {std::make_shared<Symbol>("concat"),
                                 (*subl)->at(1), list});
                            list.swap(new_list);
                            continue;
                        }
                    }
                }
            }
            auto new_list = std::make_shared<List>();
            (*new_list)->assign(
                {std::make_shared<Symbol>("cons"), Quasiquote(*it), list});
            list.swap(new_list);
        }
        if (auto v = std::dynamic_pointer_cast<Vector>(seq)) {
            auto new_list = std::make_shared<List>();
            (*new_list)->assign({std::make_shared<Symbol>("vec"), list});
            return new_list;
        }
        return list;
    } else if (std::dynamic_pointer_cast<HashMap>(ast) ||
               std::dynamic_pointer_cast<Symbol>(ast)) {
        auto list = std::make_shared<List>();
        (*list)->assign({std::make_shared<Symbol>("quote"), ast});
        return list;
    }
    return ast;
}

std::shared_ptr<MalType> Eval(std::shared_ptr<MalType> ast,
                              std::shared_ptr<Env> env);
std::shared_ptr<MalType> EvalAst(const std::shared_ptr<MalType>& ast,
                                 std::shared_ptr<Env> env) {
    if (auto s = std::dynamic_pointer_cast<Symbol>(ast)) {
        return env->Get(**s);
    } else if (auto v = std::dynamic_pointer_cast<Vector>(ast)) {
        auto vector = std::make_shared<Vector>();
        for (auto& item : **v) (*vector)->emplace_back(Eval(item, env));
        return vector;
    } else if (auto l = std::dynamic_pointer_cast<List>(ast)) {
        auto list = std::make_shared<List>();
        for (auto& v : **l) (*list)->emplace_back(Eval(v, env));
        return list;
    } else if (auto h = std::dynamic_pointer_cast<HashMap>(ast)) {
        auto map = std::make_shared<HashMap>();
        for (auto& [k, v] : **h) (*map)->emplace(k, Eval(v, env));
        return map;
    }
    return ast;
}

auto Read(std::string_view str) { return ReadStr(str); }

std::shared_ptr<MalType> Eval(std::shared_ptr<MalType> ast,
                              std::shared_ptr<Env> env) {
    while (true) {
        ast = MacroExpand(ast, env);
        if (auto l = std::dynamic_pointer_cast<List>(ast)) {
            if ((*l)->empty()) return ast;
            auto it = (*l)->begin();
            if (auto s = std::dynamic_pointer_cast<Symbol>(*it)) {
                if (const auto& action = **s; action == "def!") {
                    auto symbol = **std::dynamic_pointer_cast<Symbol>(*(++it));
                    auto value = Eval(*(++it), env);
                    env->Set(std::move(symbol), value);
                    return value;
                } else if (action == "let*") {
                    env = std::make_shared<Env>(env);
                    auto bindings =
                        std::dynamic_pointer_cast<Sequence>(*(++it));
                    for (auto it = (*bindings)->begin();
                         it + 1 < (*bindings)->end(); it += 2) {
                        auto symbol = **std::dynamic_pointer_cast<Symbol>(*it);
                        auto value = Eval(*(it + 1), env);
                        env->Set(std::move(symbol), std::move(value));
                    }
                    ast = *(++it);
                    continue;
                } else if (action == "do") {
                    std::shared_ptr<MalType> res;
                    for (++it; it != (*l)->end(); it++) res = Eval(*it, env);
                    return res;
                } else if (action == "if") {
                    auto condition = Eval(*(++it), env);
                    if (std::dynamic_pointer_cast<False>(condition) ||
                        std::dynamic_pointer_cast<Nil>(condition)) {
                        if ((*l)->size() < 4) return MalType::Nil;
                        ast = *(it + 2);
                        continue;
                    } else {
                        ast = *(++it);
                        continue;
                    }
                } else if (action == "fn*") {
                    auto binds = std::dynamic_pointer_cast<Sequence>(*(++it));
                    auto ast = *(++it);
                    std::vector<std::string> params((*binds)->size());
                    std::transform(
                        (*binds)->begin(), (*binds)->end(), params.begin(),
                        [](std::shared_ptr<MalType> arg) -> std::string {
                            return **std::dynamic_pointer_cast<Symbol>(arg);
                        });
                    UserFunc::Callback f =
                        [](std::span<std::shared_ptr<MalType>> args,
                           const UserFunc& func) {
                            auto new_env = std::make_shared<Env>(
                                func.get_params(), args, func.get_env());
                            return Eval(func.get_ast(), new_env);
                        };
                    return std::make_shared<UserFunc>(ast, params, env, f);
                } else if (action == "quote") {
                    return *(++it);
                } else if (action == "quasiquoteexpand") {
                    return Quasiquote(*(++it));
                } else if (action == "quasiquote") {
                    ast = Quasiquote(*(++it));
                    continue;
                } else if (action == "defmacro!") {
                    auto symbol = **std::dynamic_pointer_cast<Symbol>(*(++it));
                    auto value =
                        std::dynamic_pointer_cast<UserFunc>(Eval(*(++it), env));
                    env->Set(std::move(symbol), value->MakeMacro());
                    return value;
                } else if (action == "macroexpand") {
                    return MacroExpand(*(++it), env);
                } else if (action == "try*") {
                    try {
                        return Eval(*(++it), env);
                    } catch (std::shared_ptr<MalType> err) {
                        if ((*l)->size() < 3) throw err;
                        auto catch_list =
                            std::dynamic_pointer_cast<List>(*(++it));
                        auto symbol = **std::dynamic_pointer_cast<Symbol>(
                            (*catch_list)->at(1));
                        env = std::make_shared<Env>(env);
                        env->Set(symbol, err);
                        ast = (*catch_list)->at(2);
                        continue;
                    }
                }
            }
            auto list = std::dynamic_pointer_cast<List>(EvalAst(l, env));
            it = (*list)->begin();
            auto args = std::span{it + 1, (*list)->end()};
            if (auto func = std::dynamic_pointer_cast<UserFunc>(*(it))) {
                ast = func->get_ast();
                auto& params = func->get_params();
                env = std::make_shared<Env>(
                    std::span{params.begin(), params.end()}, args,
                    func->get_env());
                continue;
            }
            return (*std::dynamic_pointer_cast<MalFunc>(*(it)))(args);
        }
        return EvalAst(ast, env);
    }
}

std::string Print(auto ast) { return PrStr(ast, true); }
std::string Rep(std::string_view str, std::shared_ptr<Env> env) {
    try {
        return Print(Eval(Read(str), env));
    } catch (std::shared_ptr<MalType> err) {
        fmt::print("Exception {}", err->PrStr(false));
        return "";
    }
}

int main(int argc, const char* argv[]) {
    linenoiseHistorySetMaxLen(30);
    linenoiseHistoryLoad("history.txt");

    auto env = std::make_shared<Env>();

    for (auto& [k, v] : getNS()) {
        auto f = std::make_shared<BaseFunc>(v);
        env->Set(std::string{k}, f);
    }

    env->Set("eval", std::make_shared<BaseFunc>(
                         [env = std::weak_ptr<Env>(env)](
                             std::span<std::shared_ptr<MalType>> args) {
                             return Eval(args[0], env.lock());
                         }));

    Rep("(def! not (fn* (a) (if a false true)))", env);
    Rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) "
        "\"\nnil)\")))))",
        env);
    Rep("(defmacro! cond (fn* (& xs) (if (> (count xs) 0) (list 'if (first xs) "
        "(if (> (count xs) 1) (nth xs 1) (throw \"odd number of forms to "
        "cond\")) (cons 'cond (rest (rest xs)))))))",
        env);

    auto list = std::make_shared<List>();
    if (argc > 1) {
        for (int i = 2; i < argc; i++)
            (*list)->push_back(std::make_shared<String>(argv[i]));
        env->Set("*ARGV*", std::move(list));
        Rep(fmt::format("(load-file \"{}\")", argv[1]), env);
        return 0;
    }
    env->Set("*ARGV*", std::move(list));

    char* line;
    while ((line = linenoise("user> ")) != NULL) {
        if (line[0] == '\0') continue;
        linenoiseHistoryAdd(line);
        try {
            fmt::print("{}\n", Rep(line, env));
        } catch (std::nullptr_t) {
            linenoiseFree(line);
            continue;
        }
        linenoiseFree(line);
    }
    linenoiseHistorySave("history.txt");
    return 0;
}
