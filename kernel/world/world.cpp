#include "world.h"

#include "geometry/geometry.h"
#include "light/light.h"
#include "types/types.h"
#include "world/object.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace detail {
namespace world {

GlobalObject::GlobalObject(
    LocalObject&& local_object_, const glm::vec3& shift_, const glm::mat3x3& transform_
)
    : local_object(std::move(local_object_)) {
  transform = 0;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      transform[i][j] = transform_[i][j];
    }
  }
  transform[0][3] = shift_.x;
  transform[1][3] = shift_.y;
  transform[2][3] = shift_.z;
  transform[3][3] = 1.0;
  transform = glm::transpose(transform);
}

std::vector<GlobalObject::Triangle> GlobalObject::GetTriangles() const {
  std::vector<Triangle> triangles;
  for (const TriangleInfo& triangle_keeper : local_object.GetTriangles()) {
    Triangle triangle = local_object.GetTriangle(triangle_keeper);
    triangles.push_back(triangle * transform);
  }

  return triangles;
}

std::vector<GlobalObject::Triangle>
GlobalObject::GetTrianglesForWorker(int32_t begin, int32_t end) const {
  std::vector<Triangle> triangles;
  triangles.reserve(end - begin);
  for (int32_t i = begin; i < end; ++i) {
    triangles.push_back(local_object.GetTriangle(local_object.GetTriangles()[i]) * transform);
  }
  return triangles;
}

std::vector<GlobalObject::Segment> GlobalObject::GetSegments() const {
  std::vector<Segment> segments;
  for (const SegmentKeeper& segment_keeper : local_object.GetSegments()) {
    Segment segment = local_object.GetSegment(segment_keeper);
    segments.push_back(segment * transform);
  }

  return segments;
}

int32_t GlobalObject::TrianglesCount() const {
  return local_object.GetTriangles().size();
}

const GlobalObject::Texture& GlobalObject::GetTexture() const {
  return local_object.GetTexture();
}

Material GlobalObject::GetMaterial() const {
  return local_object.GetMaterial();
}

const std::vector<GlobalObject>& World::GetObjects() const {
  return objects;
}

int32_t World::GetTrianglesCapacity() const {
  int32_t result = 0;
  for (const GlobalObject& object : objects) {
    result += object.GetTriangles().size();
  }
  return result;
}

int32_t World::GetSegmentCapacity() const {
  int32_t result = 0;
  for (const GlobalObject& object : objects) {
    result += object.GetSegments().size();
  }
  return result;
}

SceneBoundingBox World::GetBoundingBox() const {
  return bounding_box;
}

SceneBoundingBox World::MakeBoundingBox() const {
  SceneBoundingBox bb;
  bb.x_min = FLT_MAX;
  bb.x_max = -FLT_MAX;

  bb.y_min = FLT_MAX;
  bb.y_max = -FLT_MAX;

  bb.z_min = FLT_MAX;
  bb.z_max = -FLT_MAX;

  V3 w = -V3{0, 0, -1};
  V3 tmp = glm::cross({0, 1, 0}, w);
  V3 v = glm::cross(w, tmp);

  M4 light_direction_ = geometry::MakeLookAtMatrix(tmp, v, w, {0, 0, 0});

  for (const GlobalObject& object : GetObjects()) {
    for (geometry::Triangle triangle : object.GetTriangles()) {
      triangle = triangle * light_direction_;
      // printf("Z: %.5f %.5f %.5f\n", triangle.a.Z(), triangle.b.Z(), triangle.c.Z());
      bb.x_min =
          std::min(bb.x_min, std::min(triangle.a.X(), std::min(triangle.b.X(), triangle.c.X())));
      bb.x_max =
          std::max(bb.x_max, std::max(triangle.a.X(), std::max(triangle.b.X(), triangle.c.X())));

      bb.y_min =
          std::min(bb.y_min, std::min(triangle.a.Y(), std::min(triangle.b.Y(), triangle.c.Y())));
      bb.y_max =
          std::max(bb.y_max, std::max(triangle.a.Y(), std::max(triangle.b.Y(), triangle.c.Y())));

      bb.z_min =
          std::min(bb.z_min, std::min(triangle.a.Z(), std::min(triangle.b.Z(), triangle.c.Z())));
      bb.z_max =
          std::max(bb.z_max, std::max(triangle.a.Z(), std::max(triangle.b.Z(), triangle.c.Z())));
    }
  }

  bb.x_min -= 0.5;
  bb.x_max += 0.5;
  bb.y_min -= 0.5;
  bb.y_max += 0.5;
  bb.z_min -= 0.5;
  bb.z_max += 0.5;
  return bb;
}

void WorldBuilder::AddObject(GlobalObject&& object) {
  world.objects.emplace_back(std::move(object));
}

World WorldBuilder::Extract() {
  world.bounding_box = world.MakeBoundingBox();
  return std::move(world);
}

} // namespace world
} // namespace detail
