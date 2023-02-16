#include "printer.h"

std::string PrStr(const std::shared_ptr<MalType>& ast, bool print_readably) {
    if (!ast) return "";
    return ast->PrStr(print_readably);
}
