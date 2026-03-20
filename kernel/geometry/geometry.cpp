#include "geometry/geometry.h"

#include "types/types.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <glm/ext/vector_float3.hpp>
#include <vector>

namespace detail {
namespace geometry {

namespace {
inline bool IsHigher(const ScreenPoint& left, const ScreenPoint& right) {
  if (left.y > right.y) {
    return true;
  } else if (left.y == right.y && left.x >= right.x) {
    return true;
  }
  return false;
}

struct HeightComparator {
  bool operator()(const ScreenPoint& left, const ScreenPoint& right) {
    return IsHigher(left, right);
  }
};

} // namespace

Point Point::operator+(const V3& vector) const {
  return Point{coordinates + V4{vector.x, vector.y, vector.z, 0}, color};
}

Point Point::operator+=(const V3& vector) {
  *this = *this + vector;
  return *this;
}

void Point::Scale(const float& coef) {
  float w = coordinates.w;
  coordinates *= coef;
  coordinates.w = w;
}

Point Point::operator*(const M4& matrix) const {
  V4 new_coordinates = matrix * coordinates;
  return Point{new_coordinates, color};
}

Triangle Triangle::operator*(const M4& matrix) const {
  return Triangle{a * matrix, b * matrix, c * matrix};
}

Segment Segment::operator*(const M4& matrix) const {
  return Segment{a * matrix, b * matrix};
}

float Plane::operator()(const Point& point) const {
  return glm::dot(V3{point.X(), point.Y(), point.Z()}, N) + D;
}

inline void TriangleIntersectedSingle::Append(const Triangle& triangle) {
  triangles[size++] = triangle;
}

void TriangleIntersected::Merge(const TriangleIntersectedSingle& single) {
  // std::cout << "123" << std::endl;
  for (int32_t i = 0; i < single.size; ++i) {
    triangles[size++] = single.triangles[i];
  }
}

void TriangleIntersected::Clear() {
  size = 0;
}

const Triangle& TriangleIntersected::operator[](size_t i) {
  return triangles[i];
}

namespace {
struct PointContainer {
  void Append(const Point& point) { points[size++] = point; }
  bool IsEmpty() const { return size == 0; }

  inline const Point& operator[](size_t i) { return points[i]; }

  Point points[3];
  int32_t size = 0;
};
}; // namespace

TriangleIntersectedSingle IntersectTriangleWithPlane(const Triangle& triangle, const Plane& plane) {
  PointContainer insiders;
  PointContainer outsiders;

  float eps = 1e-6;
  if (plane(triangle.a) >= eps) {
    insiders.Append(triangle.a);
  } else {
    outsiders.Append(triangle.a);
  }

  if (plane(triangle.b) >= eps) {
    insiders.Append(triangle.b);
  } else {
    outsiders.Append(triangle.b);
  }

  if (plane(triangle.c) >= eps) {
    insiders.Append(triangle.c);
  } else {
    outsiders.Append(triangle.c);
  }

  if (insiders.IsEmpty()) {
    return {};
  }

  if (insiders.size == 3) {
    return {{triangle}, .size = 1};
  }
  // return {};

  if (insiders.size == 1) {

    Point intersect1 = IntersectSegmentWithPlane({insiders[0], outsiders[0]}, plane).b;
    Point intersect2 = IntersectSegmentWithPlane({insiders[0], outsiders[1]}, plane).b;

    return {Triangle{insiders[0], intersect1, intersect2}, .size = 1};
  } else if (insiders.size == 2) {

    Point intersect1 = IntersectSegmentWithPlane({insiders[0], outsiders[0]}, plane).b;
    Point intersect2 = IntersectSegmentWithPlane({insiders[1], outsiders[0]}, plane).b;

    Triangle first = {insiders[0], intersect1, intersect2};
    Triangle second = {insiders[0], insiders[1], intersect2};

    return {first, second, .size = 2};
  }
  assert(false);
}

Segment IntersectSegmentWithPlane(const Segment& segment, const Plane& plane) {
  Point first = segment.a;
  Point second = segment.b;
  float eps = 1e-6;
  if (plane(first) >= eps && plane(second) >= eps) {
    return segment;
  }

  if (plane(first) < -eps) {
    std::swap(first, second);
  }

  V3 p1 = {first.X(), first.Y(), first.Z()};
  V3 p2 = {second.X(), second.Y(), second.Z()};
  float coef = (glm::dot(plane.N, p1) + plane.D) * -1.0f / glm::dot(plane.N, p2 - p1);
  V3 intersection = p1 + (p2 - p1) * coef;

  return {
      first,
      Point{V4{intersection.x, intersection.y, intersection.z, second.W()}, .color = second.color}
  };
}

LineStatus GetLineStatus(const ScreenPoint& first, const ScreenPoint& second) {
  if (first.x == second.x) {
    return LineStatus::Vertical;
  }
  return LineStatus::NonVertical;
}

float GetTangentCoefficent(const ScreenPoint& first, const ScreenPoint& second) {
  assert(GetLineStatus(first, second) == LineStatus::NonVertical);

  assert(first.y != second.y);

  return float(first.x - second.x) / float(first.y - second.y);
}

ScreenTriangle ScreenTriangle::SortedVertex() const {
  ScreenPoint a0 = a;
  ScreenPoint b0 = b;
  ScreenPoint c0 = c;

  if (!IsHigher(a0, b0)) {
    std::swap(a0, b0);
  }
  if (!IsHigher(a0, c0)) {
    std::swap(a0, c0);
  }

  if (!IsHigher(b0, c0)) {
    std::swap(b0, c0);
  }
  return {a0, b0, c0};

  // return ScreenTriangle{vertex[0], vertex[1], vertex[2]};
}

int32_t ScreenTriangle::MinimumHeight() const {
  return std::min(a.y, std::min(b.y, c.y));
}

int32_t ScreenTriangle::MaximumHeight() const {
  return std::max(a.y, std::max(b.y, c.y));
}

namespace {
ScreenPoint DiscretizePoint(const Point& point) {
  return ScreenPoint{
      .x = int32_t(std::ceil(point.X())),
      .y = int32_t(std::ceil(point.Y())),
      .z = point.Z(),
      .color = point.color
  };
}

} // namespace

ScreenSegment DiscretizeSegment(const Segment& segment) {
  return ScreenSegment{.a = DiscretizePoint(segment.a), .b = DiscretizePoint(segment.b)};
}

ScreenTriangle DiscretizeTriangle(const Triangle& triangle) {
  return ScreenTriangle{
      .a = DiscretizePoint(triangle.a),
      .b = DiscretizePoint(triangle.b),
      .c = DiscretizePoint(triangle.c)
  };
}

M4 GetFrustumMatrix(
    HorizontalFOV horizontal_fov, AspectRatio aspect_ratio, NearPlaneDistance near_plane_distance,
    RenderDistance render_distance, RightEdgeX r, LeftEdgeX l, TopEdgeY t, BottomEdgeY b
) {
  M4 matrix;

  matrix[0][0] = 2.0 * near_plane_distance() / (r() - l());
  matrix[0][1] = 0;
  matrix[0][2] = (r() + l()) / (r() - l());
  matrix[0][3] = 0;

  matrix[1][0] = 0;
  matrix[1][1] = 2.0 * near_plane_distance() / (t() - b());
  matrix[1][2] = (t() + b()) / (t() - b());
  matrix[1][3] = 0;

  matrix[2][0] = 0;
  matrix[2][1] = 0;
  matrix[2][2] =
      -(render_distance() + near_plane_distance()) / (render_distance() - near_plane_distance());
  matrix[2][3] = -2.0 * near_plane_distance() * render_distance() /
                 (render_distance() - near_plane_distance());

  matrix[3][0] = 0;
  matrix[3][1] = 0;
  matrix[3][2] = -1.0;
  matrix[3][3] = 0;

  return glm::transpose(matrix);
}
} // namespace geometry
} // namespace detail
