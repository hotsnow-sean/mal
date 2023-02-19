#include "core.h"

#include <fmt/core.h>

#include <chrono>
#include <fstream>

#include "linenoise.h"
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
        {"nth"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto seq = std::dynamic_pointer_cast<Sequence>(args[0]);
             auto idx = std::dynamic_pointer_cast<Number>(args[1]);
             try {
                 return (*seq)->at(**idx);
             } catch (std::out_of_range) {
                 throw std::shared_ptr<MalType>(
                     std::make_shared<String>(("out of range")));
             }
         }},
        {"first"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto seq = std::dynamic_pointer_cast<Sequence>(args[0]);
             if (!seq || (*seq)->empty()) return MalType::Nil;
             return (**seq)[0];
         }},
        {"rest"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto seq = std::dynamic_pointer_cast<Sequence>(args[0]);
             auto list = std::make_shared<List>();
             if (!seq || (*seq)->empty()) return list;
             (*list)->assign((*seq)->begin() + 1, (*seq)->end());
             return list;
         }},
        {"throw"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             throw args[0];
         }},
        {"apply"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto func = std::dynamic_pointer_cast<MalFunc>(args.front());
             auto seq = std::dynamic_pointer_cast<Sequence>(args.back());
             std::vector<std::shared_ptr<MalType>> new_args;
             if (args.size() > 2)
                 new_args.assign(args.begin() + 1, args.end() - 1);
             new_args.insert(new_args.end(), (*seq)->begin(), (*seq)->end());
             return (*func)(new_args);
         }},
        {"map"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto func = std::dynamic_pointer_cast<MalFunc>(args[0]);
             auto seq = std::dynamic_pointer_cast<Sequence>(args[1]);
             auto res = std::make_shared<List>();
             (*res)->resize((*seq)->size());
             std::transform((*seq)->begin(), (*seq)->end(), (*res)->begin(),
                            [&func](std::shared_ptr<MalType> v) {
                                std::shared_ptr<MalType> vv[] = {v};
                                return (*func)(vv);
                            });
             return res;
         }},
        {"nil?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<Nil>(args[0]) ? MalType::True
                                                            : MalType::False;
         }},
        {"true?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<True>(args[0]) ? MalType::True
                                                             : MalType::False;
         }},
        {"false?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<False>(args[0]) ? MalType::True
                                                              : MalType::False;
         }},
        {"symbol?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<Symbol>(args[0]) ? MalType::True
                                                               : MalType::False;
         }},
        {"symbol"sv,
         [](MalFunc::ParamType args) {
             auto name = std::dynamic_pointer_cast<String>(args[0]);
             return std::make_shared<Symbol>(**name);
         }},
        {"keyword"sv,
         [](MalFunc::ParamType args) {
             auto str = std::dynamic_pointer_cast<String>(args[0]);
             if (str->IsKeyWord()) return str;
             std::string name{(char)0xff};
             name += **str;
             return std::make_shared<String>(std::move(name));
         }},
        {"keyword?"sv,
         [](MalFunc::ParamType args) {
             auto value = std::dynamic_pointer_cast<String>(args[0]);
             return value && value->IsKeyWord() ? MalType::True
                                                : MalType::False;
         }},
        {"vector"sv,
         [](MalFunc::ParamType args) {
             auto vec = std::make_shared<Vector>();
             (*vec)->assign(args.begin(), args.end());
             return vec;
         }},
        {"vector?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<Vector>(args[0]) ? MalType::True
                                                               : MalType::False;
         }},
        {"sequential?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<Sequence>(args[0])
                        ? MalType::True
                        : MalType::False;
         }},
        {"hash-map"sv,
         [](MalFunc::ParamType args) {
             auto map = std::make_shared<HashMap>();
             for (auto it = args.begin();
                  it != args.end() && it + 1 != args.end(); it += 2) {
                 (*map)->emplace(**std::dynamic_pointer_cast<String>(*it),
                                 *(it + 1));
             }
             return map;
         }},
        {"map?"sv,
         [](MalFunc::ParamType args) {
             return std::dynamic_pointer_cast<HashMap>(args[0])
                        ? MalType::True
                        : MalType::False;
         }},
        {"assoc"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             auto new_map = std::make_shared<HashMap>();
             std::copy((*map)->begin(), (*map)->end(),
                       std::inserter(**new_map, (*new_map)->begin()));
             for (auto it = args.begin() + 1;
                  it != args.end() && it + 1 != args.end(); it += 2) {
                 (**new_map)[**std::dynamic_pointer_cast<String>(*it)] =
                     *(it + 1);
             }
             return new_map;
         }},
        {"dissoc"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             auto new_map = std::make_shared<HashMap>();
             std::copy((*map)->begin(), (*map)->end(),
                       std::inserter(**new_map, (*new_map)->begin()));
             for (auto& item : args.subspan(1)) {
                 auto key = std::dynamic_pointer_cast<String>(item);
                 (*new_map)->erase(**key);
             }
             return new_map;
         }},
        {"get"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             if (!map) return MalType::Nil;
             auto key = std::dynamic_pointer_cast<String>(args[1]);
             auto it = (*map)->find(**key);
             if (it != (*map)->end()) return it->second;
             return MalType::Nil;
         }},
        {"contains?"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             auto key = std::dynamic_pointer_cast<String>(args[1]);
             return (*map)->contains(**key) ? MalType::True : MalType::False;
         }},
        {"keys"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             auto keys = std::make_shared<List>();
             (*keys)->reserve((*map)->size());
             for (auto& [k, v] : **map) {
                 (*keys)->push_back(std::make_shared<String>(k));
             }
             return keys;
         }},
        {"vals"sv,
         [](MalFunc::ParamType args) {
             auto map = std::dynamic_pointer_cast<HashMap>(args[0]);
             auto vals = std::make_shared<List>();
             (*vals)->reserve((*map)->size());
             for (auto& [k, v] : **map) {
                 (*vals)->push_back(v);
             }
             return vals;
         }},
        {"readline"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto prompt = std::dynamic_pointer_cast<String>(args[0]);
             if (auto line = linenoise((*prompt)->c_str())) {
                 auto input = std::make_shared<String>(line);
                 linenoiseFree(line);
                 return input;
             }
             return MalType::Nil;
         }},
        {"time-ms"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             auto cur = std::chrono::high_resolution_clock::now();
             auto now =
                 std::chrono::time_point_cast<std::chrono::milliseconds>(cur);
             return std::make_shared<Number>(now.time_since_epoch().count());
         }},
        {"meta"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             return std::dynamic_pointer_cast<MalMeta>(args[0])->get_meta();
         }},
        {"with-meta"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             return std::dynamic_pointer_cast<MalMeta>(args[0])->WithMeta(
                 args[1]);
         }},
        {"fn?"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto f = std::dynamic_pointer_cast<UserFunc>(args[0])) {
                 return f->is_macro() ? MalType::False : MalType::True;
             }
             return std::dynamic_pointer_cast<BaseFunc>(args[0])
                        ? MalType::True
                        : MalType::False;
         }},
        {"string?"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
                 return str->IsKeyWord() ? MalType::False : MalType::True;
             }
             return MalType::False;
         }},
        {"number?"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             return std::dynamic_pointer_cast<Number>(args[0]) ? MalType::True
                                                               : MalType::False;
         }},
        {"seq"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto seq = std::dynamic_pointer_cast<Sequence>(args[0])) {
                 if ((*seq)->empty()) return MalType::Nil;
                 auto list = std::dynamic_pointer_cast<List>(seq);
                 if (!list) {
                     list = std::make_shared<List>();
                     (*list)->assign((*seq)->begin(), (*seq)->end());
                 }
                 return list;
             } else if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
                 if ((*str)->empty()) return MalType::Nil;
                 auto list = std::make_shared<List>();
                 for (auto c : (**str)) {
                     (*list)->push_back(
                         std::make_shared<String>(std::string{c}));
                 }
                 return list;
             }
             return MalType::Nil;
         }},
        {"conj"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto l = std::dynamic_pointer_cast<List>(args[0])) {
                 auto list = std::make_shared<List>();
                 (*list)->assign(args.rbegin(), args.rend() - 1);
                 (*list)->insert((*list)->end(), (*l)->begin(), (*l)->end());
                 return list;
             }
             auto v = std::dynamic_pointer_cast<Vector>(args[0]);
             auto vec = std::make_shared<Vector>();
             (*vec)->assign((*v)->begin(), (*v)->end());
             (*vec)->insert((*vec)->end(), args.begin() + 1, args.end());
             return vec;
         }},
        {"macro?"sv,
         [](MalFunc::ParamType args) -> std::shared_ptr<MalType> {
             if (auto f = std::dynamic_pointer_cast<UserFunc>(args[0])) {
                 return f->is_macro() ? MalType::True : MalType::False;
             }
             return MalType::False;
         }},
    };
    return map;
}
