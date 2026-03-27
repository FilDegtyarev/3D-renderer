#pragma once
#include "world/object.h"

#include <string>

namespace detail {
namespace parser {

using LocalObject = world::LocalObject;

LocalObject
Parse(const std::string& filename, const std::string& texture_file, BackFaceCullingStatus status);
} // namespace parser
} // namespace detail
