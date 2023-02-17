#include "reader.h"

#include <fmt/core.h>

#include <vector>

#include "types.h"

using namespace std::literals;

Reader::Reader(std::list<Token> tokens) : tokens_(std::move(tokens)) {}

Reader::operator bool() const noexcept { return !tokens_.empty(); }

Token Reader::Next() {
    auto token{std::move(tokens_.front())};
    tokens_.pop_front();
    return token;
}
const Token& Reader::Peek() const { return tokens_.front(); }

std::list<Token> Tokenize(std::string_view str) {
    std::list<Token> list;
    while (!str.empty()) {
        while (!str.empty() && (::isspace(str[0]) || str[0] == ','))
            str.remove_prefix(1);
        if (str.empty()) break;
        switch (str[0]) {
            case '~':
                if (str.size() > 1 && str[1] == '@') {
                    list.emplace_back("~@");
                    str.remove_prefix(2);
                    break;
                } else {
                    [[fallthrough]];
                }
            case '[':
            case ']':
            case '{':
            case '}':
            case '(':
            case ')':
            case '\'':
            case '`':
            case '^':
            case '@':
                list.emplace_back(std::string{str[0]});
                str.remove_prefix(1);
                break;
            case '"': {
                int i = 1;
                for (; i < str.size(); i++) {
                    if (str[i] == '\\') {
                        i++;
                    } else if (str[i] == '"') {
                        break;
                    }
                }
                if (i >= str.size()) {
                    fmt::print("unbalanced\n");
                    list.emplace_back(str);
                    str.remove_prefix(str.size());
                } else {
                    list.emplace_back(str.substr(0, i + 1));
                    str.remove_prefix(i + 1);
                }
                break;
            }
            case ';':
                str.remove_prefix(str.size());
                break;
            default: {
                auto i = str.find_first_of("[]{}()'\"`,; \t\n\v\f\r");
                if (i == str.npos) i = str.size();
                list.emplace_back(str.substr(0, i));
                str.remove_prefix(i);
                break;
            }
        }
    }
    return list;
}

std::shared_ptr<MalType> ReadAtom(Reader& reader) {
    auto token = reader.Next();
    if (::isdigit(token[0]) || (token[0] == '-' && ::isdigit(token[1])))
        return std::make_shared<Number>(std::stoi(token));
    else if (token == "nil")
        return MalType::Nil;
    else if (token == "true")
        return MalType::True;
    else if (token == "false")
        return MalType::False;
    else
        return std::make_shared<Symbol>(token);
}

String ReadString(Reader& reader) {
    auto token = reader.Next();
    if (token[0] == '"') {
        std::string str;
        for (size_t i = 1; i < token.size(); i++) {
            if (token[i] == '\\') {
                if (i + 1 >= token.size()) {
                    str += '\\';
                } else {
                    i++;
                    switch (token[i]) {
                        case '"':
                            str += '"';
                            break;
                        case 'n':
                            str += '\n';
                            break;
                        case '\\':
                            str += '\\';
                            break;
                        default:
                            str += token[i];
                            break;
                    }
                }
            } else if (token[i] == '"') {
                break;
            } else {
                str += token[i];
            }
        }
        return str;
    } else if (token[0] == ':') {
        return std::string{(char)0xff} + token.substr(1);
    }
    throw "error";
}

std::shared_ptr<MalType> ReadForm(Reader& reader);
std::shared_ptr<MalType> ReadList(Reader& reader) {
    reader.Next();
    auto list = std::make_shared<List>();
    while (reader && reader.Peek() != ")") {
        (*list)->push_back(std::move(ReadForm(reader)));
    }
    if (!reader) throw "unbalanced"sv;
    reader.Next();
    return list;
}
std::shared_ptr<MalType> ReadVector(Reader& reader) {
    reader.Next();
    auto vector = std::make_shared<Vector>();
    while (reader && reader.Peek() != "]") {
        (*vector)->push_back(std::move(ReadForm(reader)));
    }
    if (!reader) throw "unbalanced"sv;
    reader.Next();
    return vector;
}

std::shared_ptr<HashMap> ReadHashMap(Reader& reader) {
    reader.Next();
    auto map = std::make_shared<HashMap>();
    while (reader && reader.Peek() != "}") {
        auto k = ReadString(reader);
        (*map)->emplace(std::move(k), ReadForm(reader));
    }
    return map;
}

std::shared_ptr<MalType> ReadForm(Reader& reader) {
    auto& token = reader.Peek();
    if (token[0] == '(')
        return ReadList(reader);
    else if (token[0] == '[')
        return ReadVector(reader);
    else if (token[0] == '{')
        return ReadHashMap(reader);
    else if (token[0] == '"' || token[0] == ':')
        return std::make_shared<String>(ReadString(reader));
    else
        return ReadAtom(reader);
}

std::shared_ptr<MalType> ReadStr(std::string_view str) {
    auto list = Tokenize(str);
    if (list.empty()) throw std::nullptr_t{};  // means to no token
    Reader reader(std::move(list));
    return ReadForm(reader);
}
