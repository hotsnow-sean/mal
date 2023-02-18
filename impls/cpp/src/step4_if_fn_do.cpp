#include <fmt/core.h>

#include <functional>
#include <string>
#include <string_view>

#include "core.h"
#include "env.h"
#include "linenoise.h"
#include "printer.h"
#include "reader.h"

using namespace std::literals;

std::shared_ptr<MalType> Eval(const std::shared_ptr<MalType>& ast,
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

std::shared_ptr<MalType> Eval(const std::shared_ptr<MalType>& ast,
                              std::shared_ptr<Env> env) {
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
                auto new_env = std::make_shared<Env>(env);
                auto bindings = std::dynamic_pointer_cast<Sequence>(*(++it));
                for (auto it = (*bindings)->begin();
                     it + 1 < (*bindings)->end(); it += 2) {
                    auto symbol = **std::dynamic_pointer_cast<Symbol>(*it);
                    auto value = Eval(*(it + 1), new_env);
                    new_env->Set(std::move(symbol), std::move(value));
                }
                return Eval(*(++it), new_env);
            } else if (action == "do") {
                std::shared_ptr<MalType> res;
                for (++it; it != (*l)->end(); it++) res = Eval(*it, env);
                return res;
            } else if (action == "if") {
                auto condition = Eval(*(++it), env);
                if (std::dynamic_pointer_cast<False>(condition) ||
                    std::dynamic_pointer_cast<Nil>(condition)) {
                    if ((*l)->size() < 4) return MalType::Nil;
                    return Eval(*(it + 2), env);
                } else {
                    return Eval(*(++it), env);
                }
            } else if (action == "fn*") {
                auto binds_list =
                    std::dynamic_pointer_cast<Sequence>((*l)->at(1));
                auto body = (*l)->at(2);
                std::vector<std::string> binds((*binds_list)->size());
                std::transform(
                    (*binds_list)->begin(), (*binds_list)->end(), binds.begin(),
                    [](std::shared_ptr<MalType> arg) -> std::string {
                        return **std::dynamic_pointer_cast<Symbol>(arg);
                    });
                auto f = [body = std::move(body), env,
                          binds = std::move(binds)](
                             std::span<std::shared_ptr<MalType>> args) {
                    auto new_env = std::make_shared<Env>(
                        std::span{binds.begin(), binds.end()}, args, env);
                    return Eval(body, new_env);
                };
                return std::make_shared<BaseFunc>(f);
            }
        }
        auto list = std::dynamic_pointer_cast<List>(EvalAst(l, env));
        it = (*list)->begin();
        auto func = std::dynamic_pointer_cast<MalFunc>(*(it++));
        return (*func)(std::span<std::shared_ptr<MalType>>(it, (*list)->end()));
    }
    return EvalAst(ast, env);
}

std::string Print(auto ast) { return PrStr(ast, true); }
std::string Rep(std::string_view str, std::shared_ptr<Env> env) {
    try {
        return Print(Eval(Read(str), env));
    } catch (std::string_view err) {
        fmt::print("{}", err);
        return "";
    } catch (std::string err) {
        fmt::print("{}", err);
        return "";
    }
}

int main() {
    linenoiseHistorySetMaxLen(30);
    linenoiseHistoryLoad("history.txt");

    auto env = std::make_shared<Env>();

    for (auto& [k, v] : getNS()) {
        auto f = std::make_shared<BaseFunc>(v);
        env->Set(std::string{k}, f);
    }

    Rep("(def! not (fn* (a) (if a false true)))", env);

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
