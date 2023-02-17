#pragma once

#include <functional>
#include <list>
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
    using FuncType = std::function<ReturnType(ParamType)>;

    MalFunc(FuncType func);

    std::string PrStr(bool print_readably) const noexcept override;

    ReturnType operator()(ParamType args) const;

private:
    FuncType func_;
};
