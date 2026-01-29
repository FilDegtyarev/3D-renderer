#pragma once
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "types/types.h"

namespace detail {
namespace geometry {

struct Point {
  int32_t x;
  int32_t y;
  double z;

  Color color;

  inline bool operator==(const Point &left) const = default;
};

enum LineStatus { Vertical, NonVertical };

LineStatus GetLineStatus(const Point &first, const Point &second);

double GetTangentCoefficent(const Point &first, const Point &second);

struct Triangle {
  Point a;
  Point b;
  Point c;

  Triangle SortedVertex() const;
  int32_t MinimumHeight() const;
  int32_t MaximumHeight() const;
};

M4 GetFrustumMatrix(HorizontalFOV horizontal_fov, AspectRatio aspect_ratio,
                    RenderDistance render_distance, RightEdgeX r, LeftEdgeX l,
                    TopEdgeY t, BottomEdgeY b);

} // namespace geometry

} // namespace detail