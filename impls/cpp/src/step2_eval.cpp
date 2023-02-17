#include <fmt/core.h>

#include <functional>
#include <string>
#include <string_view>

#include "linenoise.h"
#include "printer.h"
#include "reader.h"
#include "types.h"

using namespace std::literals;

using FuncType = std::function<int(int, int)>;
using MyEnv = std::unordered_map<std::string_view, FuncType>;

static MyEnv repl_env = {
    {"+"sv, [](int a, int b) { return a + b; }},
    {"-"sv, [](int a, int b) { return a - b; }},
    {"*"sv, [](int a, int b) { return a * b; }},
    {"/"sv, [](int a, int b) { return a / b; }},
};

class Func : public MalType {
public:
    Func(FuncType func) : func_(func) {}
    std::string PrStr(bool) const noexcept override { return "<func>"; }

    int operator()(int a, int b) { return func_(a, b); }

private:
    FuncType func_;
};

std::shared_ptr<MalType> Eval(std::shared_ptr<MalType> ast, const MyEnv& env);
std::shared_ptr<MalType> EvalAst(std::shared_ptr<MalType> ast,
                                 const MyEnv& env) {
    if (auto s = std::dynamic_pointer_cast<Symbol>(ast)) {
        try {
            return std::make_shared<Func>(env.at(**s));
        } catch (...) {
            throw "error"sv;
        }
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

std::shared_ptr<MalType> Eval(std::shared_ptr<MalType> ast, const MyEnv& env) {
    if (auto l = std::dynamic_pointer_cast<List>(ast)) {
        if (std::dynamic_pointer_cast<Vector>(l)) return EvalAst(ast, env);
        if ((*l)->empty()) return ast;
        auto list = std::dynamic_pointer_cast<List>(EvalAst(l, env));
        auto it = (**list).begin();
        auto func = std::dynamic_pointer_cast<Func>(*(it++));
        auto num1 = **std::dynamic_pointer_cast<Number>(*(it++));
        auto num2 = **std::dynamic_pointer_cast<Number>(*(it++));
        return std::make_shared<Number>((*func)(num1, num2));
    }
    return EvalAst(ast, env);
}

std::string Print(auto ast) { return PrStr(ast, true); }
std::string Rep(std::string_view str) {
    try {
        return Print(Eval(Read(str), repl_env));
    } catch (std::string_view err) {
        fmt::print("{}", err);
        return "";
    }
}

int main() {
    linenoiseHistorySetMaxLen(30);
    linenoiseHistoryLoad("history.txt");

    char* line;
    while ((line = linenoise("user> ")) != NULL) {
        if (line[0] == '\0') continue;
        linenoiseHistoryAdd(line);
        try {
            fmt::print("{}\n", Rep(line));
        } catch (std::nullptr_t) {
            linenoiseFree(line);
            continue;
        }
        linenoiseFree(line);
    }
    linenoiseHistorySave("history.txt");
    return 0;
}
