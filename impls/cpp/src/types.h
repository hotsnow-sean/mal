#pragma once

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

class MalType {
public:
    virtual ~MalType() = default;

    virtual std::string PrStr(bool print_readably) const noexcept = 0;

    static std::shared_ptr<MalType> Nil;
    static std::shared_ptr<MalType> True;
    static std::shared_ptr<MalType> False;
};

class Symbol : public MalType {
public:
    Symbol(std::string name);

    std::string PrStr(bool print_readably) const noexcept override;

private:
    std::string name_;
};

class Number : public MalType {
public:
    Number(int value);

    std::string PrStr(bool print_readably) const noexcept override;

private:
    int value_;
};

class List : public MalType {
public:
    List(std::list<std::shared_ptr<MalType>> list);

    std::string PrStr(bool print_readably) const noexcept override;

private:
    std::list<std::shared_ptr<MalType>> list_;
};

class Vector : public MalType {
public:
    Vector(std::list<std::shared_ptr<MalType>> list);

    std::string PrStr(bool print_readably) const noexcept override;

private:
    std::list<std::shared_ptr<MalType>> list_;
};

class String : public MalType {
    friend struct Hasher;

public:
    String(std::string value);

    std::string PrStr(bool print_readably) const noexcept override;

    bool operator==(const String& other) const;

private:
    std::string value_;
};

struct Hasher {
    size_t operator()(const String& s) const;
};

class HashMap : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;

    std::unordered_map<String, std::shared_ptr<MalType>, Hasher>* operator->();

private:
    std::unordered_map<String, std::shared_ptr<MalType>, Hasher> map_;
};

class Nil : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
};
class True : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
};
class False : public MalType {
public:
    std::string PrStr(bool print_readably) const noexcept override;
};
