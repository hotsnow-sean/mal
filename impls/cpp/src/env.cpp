#include "env.h"

#include <fmt/format.h>

Env::Env(std::shared_ptr<Env> outer) : outer_(std::move(outer)) {}

void Env::Set(const std::string& symbol,
              std::shared_ptr<MalType> value) noexcept {
    data_[symbol] = std::move(value);
}

std::shared_ptr<const Env> Env::Find(const std::string& symbol) const {
    if (data_.contains(symbol)) return shared_from_this();
    if (outer_) return outer_->Find(symbol);
    return nullptr;
}

std::shared_ptr<MalType> Env::Get(const std::string& symbol) const {
    auto env = Find(symbol);
    if (env) return env->data_.at(symbol);
    throw fmt::format("'{}' not found.", symbol);
}
