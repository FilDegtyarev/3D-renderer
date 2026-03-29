#include "geometry/geometry.h"

#include "types/types.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <glm/ext/vector_float3.hpp>

namespace detail {
namespace geometry {

namespace {
inline bool IsHigher(const ScreenPoint* left, const ScreenPoint* right) {
  return (left->y > right->y) | (left->y == right->y && left->x >= right->x);
  // if () {
  //   return true;
  // } else if () {
  //   return true;
  // }
  // return false;
}

} // namespace

Point Point::operator+(const V3& vector) const {
  return Point{
      coordinates + V4{vector.x, vector.y, vector.z, 0}, .texture_coordinates = texture_coordinates
  };
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

Segment Segment::operator*(const M4& matrix) const {
  return Segment{a * matrix, b * matrix};
}

float Plane::operator()(const Point& point) const {
  return glm::dot(V3{point.X(), point.Y(), point.Z()}, N) + D;
}

inline void TriangleIntersectedSingle::Append(const Triangle& triangle) {
  triangles[size++] = triangle;
}

namespace {

struct PointContainer {
  void Append(const Point& point) { points[size++] = point; }
  bool IsEmpty() const { return size == 0; }

  inline const Point& operator[](size_t i) { return points[i]; }

  Point points[3];
  int32_t size = 0;
};

struct FastPointContainer {
  inline int16_t operator[](int16_t count) {
    int16_t accumulate = -1;
    for (int16_t i = 0; i < 3; ++i) {
      accumulate += ((mask & (1 << i)) >> i);
      if (accumulate == count) {
        return i;
      }
    }
    exit(777);
  }

  int16_t mask = 0;
  int16_t size = 0;
};

}; // namespace

static const float eps = 1e-6;

/// Нет интерполяции нормалей!!!!
namespace {
inline bool IsClippingNotRequired(const Plane& plane, const Triangle& triangle) {
  return plane(triangle.a) >= eps && plane(triangle.b) >= eps && plane(triangle.c) >= eps;
}

inline const Point& Get(const Triangle& triangle, int i) {
  // return *((Point*)(&triangle) + i);
  if (i == 0) {
    return triangle.a;
  } else if (i == 1) {
    return triangle.b;
  } else {
    return triangle.c;
  }
}

} // namespace

TriangleIntersectedSingle IntersectTriangleWithPlane(const Triangle& triangle, const Plane& plane) {
  if (IsClippingNotRequired(plane, triangle)) {
    return {{triangle}, 1};
  }

  // PointContainer insiders;
  // PointContainer outsiders;
  V3 normal = triangle.a.normal;
  uint8_t model_index = triangle.model_index;

  FastPointContainer insiders = {0, 0};
  FastPointContainer outsiders = {0, 0};

  if (plane(triangle.a) >= eps) {
    // insiders.Append(triangle.a);
    insiders.mask |= (1 << 0);
    insiders.size++;
  } else {
    // outsiders.Append(triangle.a);
    outsiders.mask |= (1 << 0);
    outsiders.size++;
  }

  if (plane(triangle.b) >= eps) {
    // insiders.Append(triangle.a);
    insiders.mask |= (1 << 1);
    insiders.size++;
  } else {
    // outsiders.Append(triangle.a);
    outsiders.mask |= (1 << 1);
    outsiders.size++;
  }

  if (plane(triangle.c) >= eps) {
    // insiders.Append(triangle.a);
    insiders.mask |= (1 << 2);
    insiders.size++;
  } else {
    // outsiders.Append(triangle.a);
    outsiders.mask |= (1 << 2);
    outsiders.size++;
  }

  if (!insiders.size) {
    return {};
  }

  if (insiders.size == 1) {
    const Point& insiders_0 = Get(triangle, insiders[0]);
    const Point& outsiders_0 = Get(triangle, outsiders[0]);
    const Point& outsiders_1 = Get(triangle, outsiders[1]);
    // Point intersect1 = IntersectSegmentWithPlane({insiders[0], outsiders[0]}, plane).b;
    // Point intersect2 = IntersectSegmentWithPlane({insiders[0], outsiders[1]}, plane).b;
    Point intersect1 = IntersectSegmentWithPlane({insiders_0, outsiders_0}, plane).b;
    Point intersect2 = IntersectSegmentWithPlane({insiders_0, outsiders_1}, plane).b;
    intersect1.normal = normal;
    intersect2.normal = normal;

    return {Triangle{insiders_0, intersect1, intersect2, .model_index = model_index}, .size = 1};
    // return {Triangle{insiders[0], intersect1, intersect2}, .size = 1};
  } else {
    const Point& insiders_0 = Get(triangle, insiders[0]);
    const Point& insiders_1 = Get(triangle, insiders[1]);
    const Point& outsiders_0 = Get(triangle, outsiders[0]);

    // Point intersect1 = IntersectSegmentWithPlane({insiders[0], outsiders[0]}, plane).b;
    // Point intersect2 = IntersectSegmentWithPlane({insiders[1], outsiders[0]}, plane).b;
    Point intersect1 = IntersectSegmentWithPlane({insiders_0, outsiders_0}, plane).b;
    Point intersect2 = IntersectSegmentWithPlane({insiders_1, outsiders_0}, plane).b;
    intersect1.normal = normal;
    intersect2.normal = normal;

    Triangle first = {insiders_0, intersect1, intersect2, .model_index = model_index};
    Triangle second = {insiders_0, insiders_1, intersect2, .model_index = model_index};

    return {first, second, 2};
  }

  exit(666);
  return {};
}

Segment IntersectSegmentWithPlane(const Segment& segment, const Plane& plane) {
  Point first = segment.a;
  Point second = segment.b;
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

  return {first, Point{V4{intersection.x, intersection.y, intersection.z, second.W()}}};
}

float GetTangentCoefficent(const ScreenPoint& first, const ScreenPoint& second) {
  assert(GetLineStatus(first, second) == LineStatus::NonVertical);

  assert(first.y != second.y);

  return float(first.x - second.x) / float(first.y - second.y);
}

ScreenTriangle ScreenTriangle::SortedVertex() const {
  const ScreenPoint* a0 = &a;
  const ScreenPoint* b0 = &b;
  const ScreenPoint* c0 = &c;

  if (!IsHigher(a0, b0)) {
    std::swap(a0, b0);
  }
  if (!IsHigher(a0, c0)) {
    std::swap(a0, c0);
  }

  if (!IsHigher(b0, c0)) {
    std::swap(b0, c0);
  }
  return {*a0, *b0, *c0};
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
      .x = static_cast<int16_t>(std::ceil(point.X())),
      .y = static_cast<int16_t>(std::ceil(point.Y())),
      .z = point.Z(),
      .texture_coordinates = point.texture_coordinates
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
      .c = DiscretizePoint(triangle.c),
      .model_index = triangle.model_index
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
