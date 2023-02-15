#include <fmt/core.h>

#include <string>
#include <string_view>

#include "linenoise.h"

std::string Read(std::string_view str) { return std::string(str); }
std::string Eval(std::string_view str) { return std::string(str); }
std::string Print(std::string_view str) { return std::string(str); }
std::string Rep(std::string_view str) { return Print(Eval(Read(str))); }

int main() {
    linenoiseHistorySetMaxLen(30);
    linenoiseHistoryLoad("history.txt");

    char *line;
    while ((line = linenoise("user> ")) != NULL) {
        if (line[0] == '\0') continue;
        linenoiseHistoryAdd(line);
        fmt::print("{}\n", Rep(line));
        linenoiseFree(line);
    }
    linenoiseHistorySave("history.txt");
    return 0;
}
