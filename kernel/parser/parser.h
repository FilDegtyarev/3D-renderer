#pragma once
#include "types/types.h"
#include "world/world.h"
#include <string>

namespace detail {
namespace parser {

std::unique_ptr<world::LocalObject> Parse(const std::string &filename);
} // namespace parser
} // namespace detail
