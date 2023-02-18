#include "env.h"

#include <fmt/format.h>

#include "types.h"

Env::Env(std::shared_ptr<Env> outer) : outer_(std::move(outer)) {}

Env::Env(std::span<const std::string> binds,
         std::span<std::shared_ptr<MalType>> exprs, std::shared_ptr<Env> outer)
    : outer_(std::move(outer)) {
    auto bit = binds.begin();
    auto eit = exprs.begin();
    for (; bit != binds.end(); bit++, eit++) {
        if (*bit == "&") {
            auto list = std::make_shared<List>();
            (*list)->assign(eit, exprs.end());
            Set(*(++bit), list);
            break;
        }
        Set(*bit, *eit);
    }
}

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
    throw std::shared_ptr<MalType>(
        std::make_shared<String>(fmt::format("'{}' not found", symbol)));
}
