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
  double w = 1.0;
  Color color;

  Point &operator=(const Point &point) = default;

  Point operator+(const V3 &vector) const;
  Point operator+=(const V3 &vector);

  void Scale(const double &coef);
  Point operator*(const M4 &matrix) const;

  bool operator==(const Point &other) const = default;
};

struct Triangle {
  Point a;
  Point b;
  Point c;

  Triangle operator*(const M4 &matrix) const;
};

struct Segment {
  Point a;
  Point b;

  Segment operator*(const M4 &matrix) const;
};

struct Plane {
  V3 N;
  double D;

  double operator()(const Point &point) const;
};

struct TriangleIntersected {
  TriangleIntersected();

  void Append(const Triangle &triangle);
  void Clear();

  Triangle triangles[64];
  int32_t size;
};

std::vector<Triangle> IntersectTriangleWithPlane(const Triangle &triangle, const Plane &plane);

// Какой отрезок получится, если пересечь с плоскостью?
Segment IntersectSegmentWithPlane(const Segment &segment, const Plane &plane);

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

double GetTangentCoefficent(const ScreenPoint &first, const ScreenPoint &second);

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
                    NearPlaneDistance near_plane_distance, RenderDistance render_distance,
                    RightEdgeX r, LeftEdgeX l, TopEdgeY t, BottomEdgeY b);

} // namespace geometry

} // namespace detail
