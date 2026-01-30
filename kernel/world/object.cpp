#include "object.h"

namespace detail {
namespace world {
LocalObject::LocalObject(const std::vector<geometry::Triangle> &triangles,
                         const std::vector<geometry::Segment> &segments)
    : triangles(triangles), segments(segments) {};

LocalObject::LocalObject(std::vector<geometry::Triangle> &&triangles)
    : triangles(std::move(triangles)) {};

const std::vector<geometry::Triangle> &LocalObject::GetTriangles() const {
  return triangles;
}

const std::vector<geometry::Segment> &LocalObject::GetSegments() const {
  return segments;
}

} // namespace world
} // namespace detail