#include "world.h"
#include "world/object.h"

namespace detail {
namespace world {

GlobalObject::GlobalObject(LocalObject &&local_object_, const glm::vec3 &shift_,
                           const glm::mat3x3 &transform_)
    : local_object(std::move(local_object_)) {
  if (transform_ != M3{1}) {
    assert(false);
  }

  transform = 1;
  transform[0][3] = shift_.x;
  transform[1][3] = shift_.y;
  transform[2][3] = shift_.z;
  transform[3][3] = 1.0;
  transform = glm::transpose(transform);
}

std::vector<geometry::Triangle> GlobalObject::GetTriangles() const {

  std::vector<geometry::Triangle> triangles;
  for (const TriangleKeeper &triangle_keeper : local_object.GetTriangles()) {
    geometry::Triangle triangle = local_object.GetTriangle(triangle_keeper);
    triangles.push_back(triangle * transform);
  }

  return triangles;
}

std::vector<geometry::Segment> GlobalObject::GetSegments() const {
  std::vector<geometry::Segment> segments;
  for (const SegmentKeeper &segment_keeper : local_object.GetSegments()) {
    geometry::Segment segment = local_object.GetSegment(segment_keeper);
    segments.push_back(segment * transform);
  }

  return segments;
}

WorldBuilder::WorldBuilder() : world() {};

void WorldBuilder::AddObject(GlobalObject &&object) {
  world.objects.emplace_back(std::move(object));
}

World WorldBuilder::Extract() { return std::move(world); }

const std::vector<GlobalObject> &World::GetObjects() const { return objects; }

int32_t World::GetTrianglesCapacity() const {
  int32_t result = 0;
  for (const GlobalObject &object : objects) {
    result += object.GetTriangles().size();
  }
  return result;
}

int32_t World::GetSegmentCapacity() const {
  int32_t result = 0;
  for (const GlobalObject &object : objects) {
    result += object.GetSegments().size();
  }
  return result;
}

} // namespace world
} // namespace detail
