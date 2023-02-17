#include "types.h"

#include <fmt/format.h>

std::shared_ptr<MalType> MalType::Nil = std::make_shared<class Nil>();
std::shared_ptr<MalType> MalType::True = std::make_shared<class True>();
std::shared_ptr<MalType> MalType::False = std::make_shared<class False>();

bool MalType::operator==(const MalType& other) const { return false; }

Symbol::Symbol(std::string name) : name_(std::move(name)) {}
std::string Symbol::PrStr(bool print_readably) const noexcept { return name_; }
const std::string& Symbol::operator*() const noexcept { return name_; }
std::string* Symbol::operator->() noexcept { return &name_; }

Number::Number(int value) : value_(value) {}
std::string Number::PrStr(bool print_readably) const noexcept {
    return fmt::format("{}", value_);
}
bool Number::operator==(const MalType& other) const {
    if (auto o = dynamic_cast<const Number*>(&other)) {
        return o->value_ == value_;
    }
    return false;
}
int Number::operator*() const noexcept { return value_; }

bool Sequence::operator==(const MalType& other) const {
    if (auto o = dynamic_cast<const Sequence*>(&other)) {
        if (o->list_.size() != list_.size()) return false;
        for (int i = 0; i < list_.size(); i++)
            if (*(o->list_[i]) != *(list_[i])) return false;
        return true;
    }
    return false;
}
const List::value_type& Sequence::operator*() const noexcept { return list_; }
List::value_type* Sequence::operator->() noexcept { return &list_; }

std::string List::PrStr(bool print_readably) const noexcept {
    std::string str{"("};
    for (auto it = list_.begin(); it != list_.end(); it++) {
        if (it != list_.begin()) str += ' ';
        str += (*it)->PrStr(print_readably);
    }
    str += ')';
    return str;
}

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
        return fmt::format("{}", value_);
    }
}
bool String::operator==(const MalType& other) const {
    if (auto o = dynamic_cast<const String*>(&other)) {
        return o->value_ == value_;
    }
    return false;
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
const HashMap::value_type& HashMap::operator*() const noexcept { return map_; }
HashMap::value_type* HashMap::operator->() noexcept { return &map_; }

std::string Nil::PrStr(bool print_readably) const noexcept { return "nil"; }
bool Nil::operator==(const MalType& other) const {
    return dynamic_cast<const Nil*>(&other);
}
std::string True::PrStr(bool print_readably) const noexcept { return "true"; }
bool True::operator==(const MalType& other) const {
    return dynamic_cast<const True*>(&other);
}
std::string False::PrStr(bool print_readably) const noexcept { return "false"; }
bool False::operator==(const MalType& other) const {
    return dynamic_cast<const False*>(&other);
}

MalFunc::MalFunc(FuncType func) : func_(std::move(func)) {}
MalFunc::ReturnType MalFunc::operator()(ParamType args) const {
    return func_(args);
}
std::string MalFunc::PrStr(bool print_readably) const noexcept {
    return "#<function>";
}
