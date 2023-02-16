#include "types.h"

#include <fmt/ostream.h>

#include <cassert>

Symbol::Symbol(std::string name) : name_(std::move(name)) {}
std::string Symbol::PrStr() const { return name_; }

Number::Number(int value) : value_(value) {}
std::string Number::PrStr() const { return fmt::format("{}", value_); }

List::List(std::list<std::shared_ptr<MalType>> list) : list_(std::move(list)) {}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<MalType>& v) {
    assert(v);
    return os << v->PrStr();
}
template <>
struct fmt::formatter<std::shared_ptr<MalType>> : ostream_formatter {};
std::string List::PrStr() const {
    return fmt::format("({})", fmt::join(list_, " "));
}
