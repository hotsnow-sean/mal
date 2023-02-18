#include "core.h"

#include <fmt/core.h>

#include <fstream>

#include "reader.h"

using namespace std::literals;

const std::unordered_map<std::string_view, BaseFunc::FuncType>& getNS() {
    static std::unordered_map<std::string_view, BaseFunc::FuncType> map{
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
        {"read-string"sv,
         [](MalFunc::ParamType args) {
             const auto& str = **std::dynamic_pointer_cast<String>(args[0]);
             return ReadStr(str);
         }},
        {"slurp"sv,
         [](MalFunc::ParamType args) {
             const auto& filename =
                 **std::dynamic_pointer_cast<String>(args[0]);
             std::ifstream ifs(filename);
             std::string content{std::istreambuf_iterator<char>(ifs),
                                 std::istreambuf_iterator<char>()};
             return std::make_shared<String>(content);
         }},
        {"atom"sv,
         [](MalFunc::ParamType args) {
             return std::make_shared<Atom>(args[0]);
         }},
        {"atom?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<Atom>(args[0]) ? MalType::True
                                                             : MalType::False;
         }},
        {"deref"sv,
         [](MalFunc::ParamType args) {
             return **std::dynamic_pointer_cast<Atom>(args[0]);
         }},
        {"reset!"sv,
         [](MalFunc::ParamType args) {
             auto value = args[1];
             **std::dynamic_pointer_cast<Atom>(args[0]) = value;
             return value;
         }},
        {"swap!"sv,
         [](MalFunc::ParamType args) {
             auto fn = std::dynamic_pointer_cast<MalFunc>(args[1]);
             auto atom = std::dynamic_pointer_cast<Atom>(args[0]);
             std::vector<std::shared_ptr<MalType>> new_args{**atom};
             if (args.size() > 2)
                 new_args.insert(new_args.end(), args.begin() + 2, args.end());
             auto value = (*fn)(new_args);
             (**atom) = value;
             return value;
         }},
        {"cons"sv,
         [](MalFunc::ParamType args) {
             auto list = std::dynamic_pointer_cast<Sequence>(args[1]);
             auto new_list = std::make_shared<List>();
             (*new_list)->push_back(args[0]);
             (*new_list)->insert((*new_list)->end(), (*list)->begin(),
                                 (*list)->end());
             return new_list;
         }},
        {"concat"sv,
         [](MalFunc::ParamType args) {
             auto new_list = std::make_shared<List>();
             for (auto& arg : args) {
                 auto list = std::dynamic_pointer_cast<Sequence>(arg);
                 (*new_list)->insert((*new_list)->end(), (*list)->begin(),
                                     (*list)->end());
             }
             return new_list;
         }},
        {"vec"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto v = std::dynamic_pointer_cast<Vector>(args[0]))
                 return args[0];
             auto list = std::dynamic_pointer_cast<List>(args[0]);
             auto vector = std::make_shared<Vector>();
             (*vector)->assign((*list)->begin(), (*list)->end());
             return vector;
         }},
    };
    return map;
}
