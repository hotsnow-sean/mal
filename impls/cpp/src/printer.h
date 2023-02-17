#pragma once

#include <memory>

class MalType;
std::string PrStr(const std::shared_ptr<MalType>& ast, bool print_readably);
