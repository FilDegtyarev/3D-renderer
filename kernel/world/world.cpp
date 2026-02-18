#include "world.h"

namespace detail {
namespace world {

// Global object хранит только базовый объект и изначальное преобразование, все
// остальное сами делаем
GlobalObject::GlobalObject(std::unique_ptr<LocalObject> &&local_object_, const glm::vec3 &shift_, const glm::mat3x3 &transform_) {
  if (transform_ != M3{1}) {
    assert(false);
  }

  // transform = 0;
  // for (int i = 0; i < 3; ++i) {
  //   for (int j = 0; j < 3; ++j) {
  //     transform[i][j] = transform_[i][j];
  //   }
  // }
  transform = 1;
  transform[0][3] = shift_.x;
  transform[1][3] = shift_.y;
  transform[2][3] = shift_.z;
  transform[3][3] = 1.0;
  transform = glm::transpose(transform);
  local_object = std::move(local_object_);
}

std::vector<geometry::Triangle> GlobalObject::GetTriangles() const {
  // std::cout << "matrix:\n";
  // for (int i = 0; i < 4; ++i) {
  //   for (int j = 0; j < 4; ++j) {
  //     std::cout << transform[i][j] << "\t";
  //   }
  //   std::cout << std::endl;
  // }
  // std::cout << "--------------\n";

  std::vector<geometry::Triangle> triangles;
  for (const TriangleKeeper &triangle_keeper : local_object->GetTriangles()) {
    geometry::Triangle triangle = local_object->GetTriangle(triangle_keeper);

    // TriangleTransform(triangle, transform);
    // TriangleShift(triangle, shift);
    triangles.push_back(triangle * transform);
  }

  return triangles;
}

std::vector<geometry::Segment> GlobalObject::GetSegments() const {
  std::vector<geometry::Segment> segments;
  for (const SegmentKeeper &segment_keeper : local_object->GetSegments()) {
    geometry::Segment segment = local_object->GetSegment(segment_keeper);
    segments.push_back(segment * transform);
  }

  return segments;
}

// GlobalObject &GlobalObject::operator*=(const M4 &matrix) {
//   transform = matrix * transform;
//   return *this;
// }

WorldBuilder::WorldBuilder() : world(new World()) {};

void WorldBuilder::AddObject(GlobalObject &&object) { world->objects.emplace_back(std::move(object)); }

std::unique_ptr<World> WorldBuilder::Extract() { return std::move(world); }

const std::vector<GlobalObject> &World::GetObjects() const { return objects; }

// void World::AddTransformation(const M4 &matrix) {
//   for (GlobalObject &obj : objects) {
//     obj *= matrix;
//   }
// }

} // namespace world
} // namespace detail
