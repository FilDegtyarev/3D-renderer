#pragma once
#include "types/types.h"
#include "world/object.h"
#include "world/world.h"
#include <string>

namespace detail {
namespace parser {

world::LocalObject Parse(const std::string &filename);
} // namespace parser
} // namespace detail
