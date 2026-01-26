#include "object.h"

namespace detail {
namespace world {
LocalObject::LocalObject(const std::vector<geometry::Triangle> &triangles)
    : triangles(triangles) {};

LocalObject::LocalObject(std::vector<geometry::Triangle> &&triangles)
    : triangles(std::move(triangles)) {};

} // namespace world
} // namespace detail