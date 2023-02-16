#pragma once

#include <memory>
#include <string>

#include "types.h"

using Token = std::string;

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
