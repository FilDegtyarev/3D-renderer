#pragma once
#include "world/object.h"

#include <string>

namespace detail {
namespace parser {

world::LocalObject Parse(const std::string& filename, const std::string& texture_file);
} // namespace parser
} // namespace detail
