#include "world.h"

namespace detail {
namespace world {

GlobalObject::GlobalObject(std::unique_ptr<LocalObject> &&local_object_,
                           const glm::vec3 &shift_,
                           const glm::mat3x3 &transform_)
    : shift(shift_), transform(transform_) {
  local_object = std::move(local_object_);
}

namespace {
inline void TriangleShift(geometry::Triangle &triangle, const V3 &vector) {
  triangle.a += vector;
  triangle.b += vector;
  triangle.c += vector;
}

inline void TriangleTransform(geometry::Triangle &triangle, const M3 &matrix) {
  triangle.a *= matrix;
  triangle.b *= matrix;
  triangle.c *= matrix;
}

inline void SegmentShift(geometry::Segment &segment, const V3 &vector) {
  segment.a += vector;
  segment.b += vector;
}

inline void SegmentTransform(geometry::Segment &segment, const M3 &matrix) {
  segment.a *= matrix;
  segment.b *= matrix;
}
} // namespace

std::vector<geometry::Triangle> GlobalObject::GetTriangles() const {
  std::vector<geometry::Triangle> triangles;
  for (const TriangleKeeper &triangle_keeper : local_object->GetTriangles()) {
    geometry::Triangle triangle = local_object->GetTriangle(triangle_keeper);
    TriangleTransform(triangle, transform);
    TriangleShift(triangle, shift);
    triangles.push_back(triangle);
  }

  return triangles;
}

std::vector<geometry::Segment> GlobalObject::GetSegments() const {
  std::vector<geometry::Segment> segments;
  for (const SegmentKeeper &segment_keeper : local_object->GetSegments()) {
    geometry::Segment segment = local_object->GetSegment(segment_keeper);
    SegmentTransform(segment, transform);
    SegmentShift(segment, shift);
    segments.push_back(segment);
  }

  return segments;
}

// GlobalObject GlobalObject::operator+(const V3 &vector) const {
//   return GlobalObject(local_object, vector + shift, transform);
// }

GlobalObject &GlobalObject::operator+=(const V3 &vector) {
  shift += vector;
  return *this;
}

// GlobalObject GlobalObject::operator*(const M3 &matrix) const {
//   return GlobalObject(local_object, shift, transform * matrix);
// }

GlobalObject &GlobalObject::operator*=(const M3 &matrix) {
  transform *= matrix;
  return *this;
}

WorldBuilder::WorldBuilder() : world(new World()) {};

void WorldBuilder::AddObject(GlobalObject &&object) {
  world->objects.emplace_back(std::move(object));
}

std::unique_ptr<World> WorldBuilder::Extract() { return std::move(world); }

const std::vector<GlobalObject> &World::GetObjects() const { return objects; }

} // namespace world
} // namespace detail
