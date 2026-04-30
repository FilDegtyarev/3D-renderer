#include "shadow_map.h"

#include "geometry/geometry.h"
#include "types/types.h"

#include <cstdio>
#include <vector>
/*
{
  x_min = -2;
  x_max = 2;
  y_min = -2;
  y_max = 2;
  z_max = 10;
}
*/
namespace detail {
namespace shadow_mapping {
M4 MakeWorldToLightMatrix(const V3& direction_light, SceneBoundingBox bounding_box) {
  float x_min, x_max, y_min, y_max, z_max;
  {
    x_min = bounding_box.x_min;
    x_max = bounding_box.x_max;
    y_min = bounding_box.y_min;
    y_max = bounding_box.y_max;
  }

  z_max = std::abs(bounding_box.z_min);

  M4 light_proj = geometry::MakeProjMatrix(x_min, x_max, y_min, y_max, z_max);

  V3 w = -direction_light;
  V3 tmp = glm::cross({0, 1, 0}, w);
  V3 v = glm::cross(w, tmp);

  M4 light_space = geometry::MakeLookAtMatrix(tmp, v, w, {0, 0, 0});

  M4 world_to_light = light_proj * light_space;
  return world_to_light;
}

using Plane = geometry::Plane;
static std::vector<Plane> planes = {Plane{V3{0, 0, 1}, 1}, Plane{V3{0, 0, -1}, 1},
                                    Plane{V3{1, 0, 0}, 1}, Plane{V3{-1, 0, 0}, 1},
                                    Plane{V3{0, 1, 0}, 1}, Plane{V3{0, -1, 0}, 1}};

TriangleIntersected*
ClipTriangle(const Triangle& triangle, TriangleIntersected* cur, TriangleIntersected* prev) {
  cur->Clear();
  prev->Clear();

  (*prev)[0] = triangle;
  prev->size = 1;
  geometry::TriangleIntersectedSingle buffer;

  for (const Plane& plane : planes) {

    for (int32_t triangle_index = 0; triangle_index < (*prev).size; ++triangle_index) {
      if (geometry::IntersectTriangleWithPlane((*prev)[triangle_index], plane, &buffer) ==
          geometry::IntersectionStatus::NotRequired) {
        cur->triangles[cur->size] = (*prev)[triangle_index];
        cur->size++;
      } else {
        cur->Merge(buffer);
      }
      // cur->Merge(geometry::IntersectTriangleWithPlane((*prev)[triangle_index], plane));
    }
    std::swap(prev, cur);
    cur->Clear();
  }

  return prev;
}

} // namespace shadow_mapping
} // namespace detail
