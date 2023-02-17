#pragma once

#include <list>
#include <memory>
#include <string>

using Token = std::string;

class MalType;
class Reader {
public:
    Reader(std::list<Token> tokens);

    operator bool() const noexcept;

    Token Next();
    const Token& Peek() const;

private:
    std::list<Token> tokens_;
};

std::shared_ptr<MalType> ReadStr(std::string_view str);
