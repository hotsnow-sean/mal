#pragma once

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

class MalType {
public:
    virtual ~MalType() = default;

    virtual std::string PrStr(bool print_readably) const noexcept = 0;
    virtual bool operator==(const MalType& other) const;

    static std::shared_ptr<MalType> Nil;
    static std::shared_ptr<MalType> True;
    static std::shared_ptr<MalType> False;
};

class Symbol : public MalType {
public:
    Symbol(std::string name);

    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;

    const std::string& operator*() const noexcept;
    std::string* operator->() noexcept;

private:
    std::string name_;
};

class Number : public MalType {
public:
    Number(int value);

    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;

    int operator*() const noexcept;

private:
    int value_;
};

class Sequence : public MalType {
public:
    using value_type = std::vector<std::shared_ptr<MalType>>;

    bool operator==(const MalType& other) const override;

    const value_type& operator*() const noexcept;
    value_type* operator->() noexcept;

protected:
    value_type list_;
};

class List : public Sequence {
public:
    std::string PrStr(bool print_readably) const noexcept override;
};

class Vector : public Sequence {
public:
    std::string PrStr(bool print_readably) const noexcept override;
};

class String : public MalType {
    friend struct Hasher;

public:
    String(std::string value);

    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;

    const std::string& operator*() const noexcept;
    std::string* operator->() noexcept;
    bool operator==(const String& other) const;

private:
    std::string value_;
};

struct Hasher {
    size_t operator()(const String& s) const;
};

class HashMap : public MalType {
public:
    using value_type =
        std::unordered_map<String, std::shared_ptr<MalType>, Hasher>;

    std::string PrStr(bool print_readably) const noexcept override;

    const value_type& operator*() const noexcept;
    value_type* operator->() noexcept;

private:
    value_type map_;
};

class Nil : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;
};
class True : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;
};
class False : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
    bool operator==(const MalType& other) const override;
};

class MalFunc : public MalType {
public:
    using ParamType = std::span<std::shared_ptr<MalType>>;
    using ReturnType = std::shared_ptr<MalType>;
    std::string PrStr(bool print_readably) const noexcept override;

    virtual ReturnType operator()(ParamType args) const = 0;
};

class BaseFunc : public MalFunc {
public:
    using FuncType = std::function<ReturnType(ParamType)>;

    BaseFunc(FuncType func);

    ReturnType operator()(ParamType args) const override;

private:
    FuncType func_;
};

class Env;
class UserFunc : public MalFunc {
public:
    using Callback = std::function<ReturnType(ParamType, const UserFunc&)>;

    UserFunc(std::shared_ptr<MalType> ast, std::vector<std::string> params,
             std::shared_ptr<Env> env, Callback callback) noexcept;

    ReturnType operator()(ParamType args) const override;

    constexpr const std::shared_ptr<MalType>& get_ast() const noexcept {
        return ast_;
    }
    constexpr const std::vector<std::string>& get_params() const noexcept {
        return params_;
    }
    constexpr const std::shared_ptr<Env>& get_env() const noexcept {
        return env_;
    }

private:
    std::shared_ptr<MalType> ast_;
    std::vector<std::string> params_;
    std::shared_ptr<Env> env_;
    Callback callback_;
};

class Atom : public MalType {
public:
    using value_type = std::shared_ptr<MalType>;

    Atom(value_type value);

    std::string PrStr(bool print_readably) const noexcept override;

    const value_type& operator*() const noexcept;
    std::shared_ptr<MalType>& operator*() noexcept;
    value_type* operator->() noexcept;

private:
    value_type value_;
};
