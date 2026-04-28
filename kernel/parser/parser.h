#pragma once
#include "world/object.h"

#include <string>

namespace detail {
namespace parser {

using LocalObject = world::LocalObject;

LocalObject Parse(
    const std::string& filename, const std::string& texture_file,
    const std::string& path_to_material, BackFaceCullingStatus status
);
} // namespace parser
} // namespace detail
