#include "types.h"

#include <fmt/format.h>

std::shared_ptr<MalType> MalType::Nil = std::make_shared<class Nil>();
std::shared_ptr<MalType> MalType::True = std::make_shared<class True>();
std::shared_ptr<MalType> MalType::False = std::make_shared<class False>();

Symbol::Symbol(std::string name) : name_(std::move(name)) {}
std::string Symbol::PrStr(bool print_readably) const noexcept { return name_; }

Number::Number(int value) : value_(value) {}
std::string Number::PrStr(bool print_readably) const noexcept {
    return fmt::format("{}", value_);
}

List::List(std::list<std::shared_ptr<MalType>> list) : list_(std::move(list)) {}
std::string List::PrStr(bool print_readably) const noexcept {
    std::string str{"("};
    for (auto it = list_.begin(); it != list_.end(); it++) {
        if (it != list_.begin()) str += ' ';
        str += (*it)->PrStr(print_readably);
    }
    str += ')';
    return str;
}

Vector::Vector(std::list<std::shared_ptr<MalType>> list)
    : list_(std::move(list)) {}
std::string Vector::PrStr(bool print_readably) const noexcept {
    std::string str{"["};
    for (auto it = list_.begin(); it != list_.end(); it++) {
        if (it != list_.begin()) str += ' ';
        str += (*it)->PrStr(print_readably);
    }
    str += ']';
    return str;
}

String::String(std::string value) : value_(value) {}
std::string String::PrStr(bool print_readably) const noexcept {
    if (!value_.empty() && value_[0] == (char)0xff) {
        return fmt::format(":{}", std::string_view(value_).substr(1));
    } else if (print_readably) {
        std::string str;
        str.reserve(value_.size());
        for (char c : value_) {
            if (c == '"')
                str += "\\\"";
            else if (c == '\n')
                str += "\\n";
            else if (c == '\\')
                str += "\\\\";
            else
                str += c;
        }
        return fmt::format("\"{}\"", str);
    } else {
        return fmt::format("\"{}\"", value_);
    }
}
bool String::operator==(const String& other) const {
    return value_ == other.value_;
}

size_t Hasher::operator()(const String& s) const {
    return std::hash<std::string>{}(s.value_);
}

std::string HashMap::PrStr(bool print_readably) const noexcept {
    std::string str{"{"};
    for (auto it = map_.begin(); it != map_.end(); it++) {
        if (it != map_.begin()) str += ' ';
        str += it->first.PrStr(print_readably);
        str += ' ';
        str += it->second->PrStr(print_readably);
    }
    str += '}';
    return str;
}
std::unordered_map<String, std::shared_ptr<MalType>, Hasher>*
HashMap::operator->() {
    return &map_;
}

std::string Nil::PrStr(bool print_readably) const noexcept { return "nil"; }
std::string True::PrStr(bool print_readably) const noexcept { return "true"; }
std::string False::PrStr(bool print_readably) const noexcept { return "false"; }
