#pragma once
#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace world {
class LocalObject {

public:
  LocalObject(const std::vector<geometry::Triangle> &triangles,
              const std::vector<geometry::Segment> &segments);

  LocalObject(std::vector<geometry::Triangle> &&triangles);

  const std::vector<geometry::Triangle> &GetTriangles() const;
  const std::vector<geometry::Segment> &GetSegments() const;

private:
  std::vector<geometry::Triangle> triangles;
  std::vector<geometry::Segment> segments;
};

} // namespace world

} // namespace detail
