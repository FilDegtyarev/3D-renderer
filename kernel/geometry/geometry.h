#pragma once
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include "types/types.h"

namespace detail {
namespace geometry {
struct Point {
  double x;
  double y;
  double z;

  Color color;

  Point &operator=(const Point &point) = default;

  Point operator+(const V3 &vector) const;
  Point operator+=(const V3 &vector);

  Point operator*(const M3 &matrix) const;
  Point operator*=(const M3 &matrix);
};

struct Triangle {
  Point a;
  Point b;
  Point c;
};

struct Segment {
  Point a;
  Point b;
};

V4 SwitchToProjective(const Point &point);

struct ScreenPoint {
  int32_t x;
  int32_t y;
  double z;

  Color color;

  inline bool operator==(const ScreenPoint &left) const = default;
};

struct ScreenSegment {
  ScreenPoint a;
  ScreenPoint b;
};

enum LineStatus { Vertical, NonVertical };

LineStatus GetLineStatus(const ScreenPoint &first, const ScreenPoint &second);

double GetTangentCoefficent(const ScreenPoint &first,
                            const ScreenPoint &second);

struct ScreenTriangle {
  ScreenPoint a;
  ScreenPoint b;
  ScreenPoint c;

  ScreenTriangle SortedVertex() const;
  int32_t MinimumHeight() const;
  int32_t MaximumHeight() const;
};

ScreenSegment DiscretizeSegment(const Segment &segment);

ScreenTriangle DiscretizeTriangle(const Triangle &triangle);

M4 GetFrustumMatrix(HorizontalFOV horizontal_fov, AspectRatio aspect_ratio,
                    NearPlaneDistance near_plane_distance,
                    RenderDistance render_distance, RightEdgeX r, LeftEdgeX l,
                    TopEdgeY t, BottomEdgeY b);

} // namespace geometry

} // namespace detail