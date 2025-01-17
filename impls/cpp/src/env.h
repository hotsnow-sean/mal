#pragma once

#include <memory>
#include <span>
#include <unordered_map>

class MalType;
class Env : public std::enable_shared_from_this<Env> {
public:
    explicit Env(std::shared_ptr<Env> outer = nullptr);
    Env(std::span<const std::string> binds,
        std::span<std::shared_ptr<MalType>> exprs,
        std::shared_ptr<Env> outer = nullptr);

    void Set(const std::string& symbol,
             std::shared_ptr<MalType> value) noexcept;
    std::shared_ptr<const Env> Find(const std::string& symbol) const;
    std::shared_ptr<MalType> Get(const std::string& symbol) const;

private:
    std::shared_ptr<Env> outer_;
    std::unordered_map<std::string, std::shared_ptr<MalType>> data_;
};
