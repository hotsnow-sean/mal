#include <fmt/core.h>

#include <string>
#include <string_view>

#include "linenoise.h"
#include "printer.h"
#include "reader.h"

auto Read(std::string_view str) {
    try {
        return ReadStr(str);
    } catch (std::string_view err) {
        fmt::print("{}", err);
        return std::shared_ptr<MalType>(nullptr);
    }
}
auto Eval(auto ast) { return ast; }
std::string Print(auto ast) { return PrStr(ast, true); }
std::string Rep(std::string_view str) { return Print(Eval(Read(str))); }

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
