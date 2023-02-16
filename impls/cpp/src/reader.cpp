#include "reader.h"

#include <vector>

#include "fmt/core.h"

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
                        i++;
                        break;
                    }
                }
                list.emplace_back(str.substr(0, i));
                str.remove_prefix(i);
                if (i >= str.size()) fmt::print("unbalanced\n");
                break;
            }
            case ';':
                list.emplace_back(str);
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
    if (::isdigit(token[0]))
        return std::make_shared<Number>(std::stoi(token));
    else
        return std::make_shared<Symbol>(token);
}

std::shared_ptr<MalType> ReadForm(Reader& reader);
std::shared_ptr<MalType> ReadList(Reader& reader) {
    reader.Next();
    std::list<std::shared_ptr<MalType>> list;
    while (reader && reader.Peek()[0] != ')') {
        list.push_back(std::move(ReadForm(reader)));
    }
    if (!reader) throw "unbalanced";
    reader.Next();
    return std::make_shared<List>(std::move(list));
}

std::shared_ptr<MalType> ReadForm(Reader& reader) {
    auto& token = reader.Peek();
    if (token[0] == '(')
        return ReadList(reader);
    else
        return ReadAtom(reader);
}

std::shared_ptr<MalType> ReadStr(std::string_view str) {
    Reader reader(Tokenize(str));
    return ReadForm(reader);
}
