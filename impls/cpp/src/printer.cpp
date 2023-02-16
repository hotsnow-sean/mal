#include "printer.h"

std::string PrStr(const std::shared_ptr<MalType>& ast) {
    if (!ast) return "";
    return ast->PrStr();
}
