#include "core.h"

#include <fmt/core.h>

using namespace std::literals;

const std::unordered_map<std::string_view, MalFunc::FuncType>& getNS() {
    static std::unordered_map<std::string_view, MalFunc::FuncType> map{
        {"+"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return std::make_shared<Number>(num1 + num2);
         }},
        {"-"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return std::make_shared<Number>(num1 - num2);
         }},
        {"*"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return std::make_shared<Number>(num1 * num2);
         }},
        {"/"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return std::make_shared<Number>(num1 / num2);
         }},
        {"prn"sv,
         [](MalFunc::ParamType args) {
             for (auto it = args.begin(); it != args.end(); it++) {
                 if (it != args.begin()) fmt::print(" ");
                 fmt::print("{}", (*it)->PrStr(true));
             }
             fmt::print("\n");
             return MalType::Nil;
         }},
        {"list"sv,
         [](MalFunc::ParamType args) {
             auto list = std::make_shared<List>();
             (*list)->assign(args.begin(), args.end());
             return list;
         }},
        {"list?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<List>(args[0]) ? MalType::True
                                                             : MalType::False;
         }},
        {"empty?"sv,
         [](MalFunc::ParamType args) {
             auto seq = std::dynamic_pointer_cast<Sequence>(args[0]);
             return (*seq)->empty() ? MalType::True : MalType::False;
         }},
        {"count"sv,
         [](MalFunc::ParamType args) {
             auto seq = std::dynamic_pointer_cast<Sequence>(args[0]);
             if (!seq) return std::make_shared<Number>(0);
             return std::make_shared<Number>((*seq)->size());
         }},
        {"="sv,
         [](MalFunc::ParamType args) {
             return *args[0] == *args[1] ? MalType::True : MalType::False;
         }},
        {"<"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return num1 < num2 ? MalType::True : MalType::False;
         }},
        {"<="sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return num1 <= num2 ? MalType::True : MalType::False;
         }},
        {">"sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return num1 > num2 ? MalType::True : MalType::False;
         }},
        {">="sv,
         [](MalFunc::ParamType args) {
             auto num1 = **std::dynamic_pointer_cast<Number>(args[0]);
             auto num2 = **std::dynamic_pointer_cast<Number>(args[1]);
             return num1 >= num2 ? MalType::True : MalType::False;
         }},
        {"pr-str"sv,
         [](MalFunc::ParamType args) {
             std::string str;
             for (auto it = args.begin(); it != args.end(); it++) {
                 if (it != args.begin()) str += ' ';
                 str += (*it)->PrStr(true);
             }
             return std::make_shared<String>(str);
         }},
        {"str"sv,
         [](MalFunc::ParamType args) {
             std::string str;
             for (auto it = args.begin(); it != args.end(); it++) {
                 str += (*it)->PrStr(false);
             }
             return std::make_shared<String>(str);
         }},
        {"println"sv,
         [](MalFunc::ParamType args) {
             for (auto it = args.begin(); it != args.end(); it++) {
                 if (it != args.begin()) fmt::print(" ");
                 fmt::print("{}", (*it)->PrStr(false));
             }
             fmt::print("\n");
             return MalType::Nil;
         }},
    };
    return map;
}
