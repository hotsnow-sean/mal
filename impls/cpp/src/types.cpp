#include "types.h"

#include <fmt/format.h>

std::shared_ptr<MalType> MalType::Nil = std::make_shared<class Nil>();
std::shared_ptr<MalType> MalType::True = std::make_shared<class True>();
std::shared_ptr<MalType> MalType::False = std::make_shared<class False>();

bool MalType::operator==(const MalType& other) const { return false; }

Symbol::Symbol(std::string name) : name_(std::move(name)) {}
std::string Symbol::PrStr(bool print_readably) const noexcept { return name_; }
bool Symbol::operator==(const MalType& other) const {
    if (auto o = dynamic_cast<const Symbol*>(&other)) {
        return o->name_ == name_;
    }
    return false;
}
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
const Sequence::value_type& Sequence::operator*() const noexcept {
    return list_;
}
Sequence::value_type* Sequence::operator->() noexcept { return &list_; }

std::string List::PrStr(bool print_readably) const noexcept {
    std::string str{"("};
    for (auto it = list_.begin(); it != list_.end(); it++) {
        if (it != list_.begin()) str += ' ';
        str += (*it)->PrStr(print_readably);
    }
    str += ')';
    return str;
}
std::shared_ptr<MalType> List::WithMeta(
    std::shared_ptr<MalType> metadata) const noexcept {
    auto meta_list = std::make_shared<List>();
    meta_list->list_ = list_;
    meta_list->metadata_ = metadata;
    return meta_list;
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
std::shared_ptr<MalType> Vector::WithMeta(
    std::shared_ptr<MalType> metadata) const noexcept {
    auto meta_vec = std::make_shared<Vector>();
    meta_vec->list_ = list_;
    meta_vec->metadata_ = metadata;
    return meta_vec;
}

String::String(std::string value) : value_(value) {}
std::string String::PrStr(bool print_readably) const noexcept {
    if (IsKeyWord()) {
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
const std::string& String::operator*() const noexcept { return value_; }
std::string* String::operator->() noexcept { return &value_; }
bool String::operator==(const String& other) const {
    return value_ == other.value_;
}
bool String::IsKeyWord() const noexcept {
    return !value_.empty() && value_[0] == (char)0xff;
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
bool HashMap::operator==(const MalType& other) const {
    auto o = dynamic_cast<const HashMap*>(&other);
    if (!o || o->map_.size() != map_.size()) return false;
    for (auto& [k, v] : o->map_) {
        auto it = map_.find(k);
        if (it == map_.end() || *v != *it->second) return false;
    }
    return true;
}
std::shared_ptr<MalType> HashMap::WithMeta(
    std::shared_ptr<MalType> metadata) const noexcept {
    auto meta_map = std::make_shared<HashMap>();
    meta_map->map_ = map_;
    meta_map->metadata_ = metadata;
    return meta_map;
}
const HashMap::value_type& HashMap::operator*() const noexcept { return map_; }
HashMap::value_type& HashMap::operator*() noexcept { return map_; }
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

std::string MalFunc::PrStr(bool print_readably) const noexcept {
    return "#<function>";
}

BaseFunc::BaseFunc(FuncType func) : func_(std::move(func)) {}
std::shared_ptr<MalType> BaseFunc::WithMeta(
    std::shared_ptr<MalType> metadata) const noexcept {
    auto meta_fun = std::make_shared<BaseFunc>(func_);
    meta_fun->metadata_ = metadata;
    return meta_fun;
}
BaseFunc::ReturnType BaseFunc::operator()(ParamType args) const {
    return func_(args);
}

UserFunc::UserFunc(std::shared_ptr<MalType> ast,
                   std::vector<std::string> params, std::shared_ptr<Env> env,
                   Callback callback) noexcept
    : ast_(std::move(ast)),
      params_(std::move(params)),
      env_(std::move(env)),
      callback_(std::move(callback)) {}
std::shared_ptr<MalType> UserFunc::WithMeta(
    std::shared_ptr<MalType> metadata) const noexcept {
    auto meta_fun = std::make_shared<UserFunc>(ast_, params_, env_, callback_);
    meta_fun->is_macro_ = is_macro_;
    meta_fun->metadata_ = metadata;
    return meta_fun;
}
UserFunc::ReturnType UserFunc::operator()(ParamType args) const {
    return callback_(args, *this);
}
std::shared_ptr<UserFunc> UserFunc::MakeMacro() const noexcept {
    auto macro_func =
        std::make_shared<UserFunc>(ast_, params_, env_, callback_);
    macro_func->is_macro_ = true;
    return macro_func;
}

Atom::Atom(value_type value) : value_(std::move(value)) {}
std::string Atom::PrStr(bool print_readably) const noexcept {
    return fmt::format("(atom {})", (*value_).PrStr(print_readably));
}
const Atom::value_type& Atom::operator*() const noexcept { return value_; }
std::shared_ptr<MalType>& Atom::operator*() noexcept { return value_; }
Atom::value_type* Atom::operator->() noexcept { return &value_; }
