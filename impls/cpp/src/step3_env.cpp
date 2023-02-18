#include <fmt/core.h>

#include <functional>
#include <string>
#include <string_view>

#include "env.h"
#include "linenoise.h"
#include "printer.h"
#include "reader.h"
#include "types.h"

using namespace std::literals;

std::unordered_map<std::string_view, std::function<int(int, int)>> repl_env = {
    {"+"sv, [](int a, int b) { return a + b; }},
    {"-"sv, [](int a, int b) { return a - b; }},
    {"*"sv, [](int a, int b) { return a * b; }},
    {"/"sv, [](int a, int b) { return a / b; }},
};

std::shared_ptr<MalType> Eval(std::shared_ptr<MalType> ast,
                              std::shared_ptr<Env> env);
std::shared_ptr<MalType> EvalAst(std::shared_ptr<MalType> ast,
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
    if (auto l = std::dynamic_pointer_cast<List>(ast)) {
        if ((*l)->empty()) return ast;
        auto it = (*l)->begin();
        if (auto s = std::dynamic_pointer_cast<Symbol>(*it)) {
            if (**s == "def!") {
                auto symbol = **std::dynamic_pointer_cast<Symbol>(*(++it));
                auto value = Eval(*(++it), env);
                env->Set(std::move(symbol), value);
                return value;
            } else if (**s == "let*") {
                auto new_env = std::make_shared<Env>(env);
                auto bindings = std::dynamic_pointer_cast<Sequence>(*(++it));
                for (auto it = (*bindings)->begin();
                     it + 1 < (*bindings)->end(); it += 2) {
                    auto symbol = **std::dynamic_pointer_cast<Symbol>(*it);
                    auto value = Eval(*(it + 1), new_env);
                    new_env->Set(std::move(symbol), std::move(value));
                }
                return Eval(*(++it), new_env);
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
    } catch (std::shared_ptr<MalType> err) {
        fmt::print("Exception {}", err->PrStr(false));
        return "";
    }
}

int main() {
    linenoiseHistorySetMaxLen(30);
    linenoiseHistoryLoad("history.txt");

    auto env = std::make_shared<Env>();

    for (auto& p : repl_env) {
        auto& v = p.second;
        auto f = std::make_shared<BaseFunc>(
            [&v](std::span<std::shared_ptr<MalType>> args) {
                auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
                auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
                return std::make_shared<Number>(v(num1, num2));
            });
        env->Set(std::string{p.first}, f);
    }

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
