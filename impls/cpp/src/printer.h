#pragma once

#include <memory>

#include "types.h"

std::string PrStr(const std::shared_ptr<MalType>& ast, bool print_readably);
