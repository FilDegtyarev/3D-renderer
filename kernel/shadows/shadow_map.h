#pragma once
#include "geometry/geometry.h"
#include "types/types.h"

namespace detail {
namespace shadow_mapping {
using TriangleIntersected = geometry::TriangleIntersected;
using Triangle = geometry::Triangle;
using Point = geometry::Point;

M4 MakeWorldToLightMatrix(const V3& direction_light, SceneBoundingBox bounding_box);

TriangleIntersected*
ClipTriangle(const Triangle& triangle, TriangleIntersected* cur, TriangleIntersected* prev);

namespace {
inline float GetByPoint(const ShadowMap* shadow_map, const Point& point) {
  return (*shadow_map)(std::round(point.Y()), std::round(point.X()));
}
} // namespace

static const float bias = 0.02;
inline Color ShadowTest(const Point& point, Color color, const ShadowMap* shadow_map) {
  float current_distance = 1 + point.Z();
  // printf("C: %.5f G: %.5f\n", current_distance, GetByPoint(shadow_map, point));
  if (current_distance - bias > GetByPoint(shadow_map, point)) {
    color = color * 0.2;
  }
  return color;
}

inline float ShadowCoefficient(const Point& point, Color color, const ShadowMap* shadow_map) {
  float current_distance = 1 + point.Z();
  if (current_distance - bias > GetByPoint(shadow_map, point)) {
    return 0.2f;
  }
  return 1.0f;
}
} // namespace shadow_mapping
} // namespace detail
