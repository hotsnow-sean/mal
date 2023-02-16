#pragma once

#include <list>
#include <memory>
#include <string>

class MalType {
public:
    virtual ~MalType() = default;

    virtual std::string PrStr() const = 0;
};

class Symbol : public MalType {
public:
    Symbol(std::string name);

    std::string PrStr() const override;

private:
    std::string name_;
};

class Number : public MalType {
public:
    Number(int value);

    std::string PrStr() const override;

private:
    int value_;
};

class List : public MalType {
public:
    List(std::list<std::shared_ptr<MalType>> list);

    std::string PrStr() const override;

private:
    std::list<std::shared_ptr<MalType>> list_;
};
