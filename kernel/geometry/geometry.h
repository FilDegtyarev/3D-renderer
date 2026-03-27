#pragma once
#include "types/types.h"

#include <algorithm>
#include <cmath>

namespace detail {
namespace geometry {
struct Point {
  Point& operator=(const Point& point) = default;

  Point operator+(const V3& vector) const;
  Point operator+=(const V3& vector);

  inline V3 operator-(const Point& vector) const {
    V3 result = coordinates - vector.coordinates;
    return result;
  }

  inline V3 operator-(const V3& vector) const {
    V3 new_coordinates(coordinates.x, coordinates.y, coordinates.z);
    return new_coordinates - vector;
  }

  void Scale(const float& coef);
  Point operator*(const M4& matrix) const;

  bool operator==(const Point& other) const = default;

  inline void Normalize() {
    float w = coordinates.w;
    coordinates /= w;
    coordinates.w = w;
  }

  inline float X() const { return coordinates.x; }
  inline float Y() const { return coordinates.y; }
  inline float Z() const { return coordinates.z; }
  inline float W() const { return coordinates.w; }
  inline V3 GetXYZ() const { return V3{coordinates.x, coordinates.y, coordinates.z}; };

  V4 coordinates = {0, 0, 0, 1};
  TextureCoordinates texture_coordinates;
};

struct Triangle {
  Triangle operator*(const M4& matrix) const;

  Point a;
  Point b;
  Point c;
  uint8_t model_index = 255;
};

inline V3 MakeNormal(const Triangle& triangle) {
  return glm::normalize(glm::cross(triangle.c - triangle.a, triangle.b - triangle.a));
}

inline bool IsBackFace(const Triangle& triangle, const V3& vector) {
  return !(
      glm::dot(triangle.a - vector, glm::cross(triangle.c - triangle.a, triangle.b - triangle.a)) >=
      1e-8
  );
}

struct Segment {
  Point a;
  Point b;

  Segment operator*(const M4& matrix) const;
};

struct Plane {
  V3 N;
  float D;

  float operator()(const Point& point) const;
};

struct TriangleIntersectedSingle {
  void Append(const Triangle& triangle);
  Triangle triangles[2];
  int32_t size;
};

struct TriangleIntersected {
  inline void Merge(const TriangleIntersectedSingle& single) {
    for (int32_t i = 0; i < single.size; ++i) {
      triangles[size++] = single.triangles[i];
    }
  }

  inline void Clear() { size = 0; }
  inline const Triangle& operator[](size_t i) const { return triangles[i]; }
  inline Triangle& operator[](size_t i) { return triangles[i]; }

  Triangle triangles[64];
  int32_t size;
};

TriangleIntersectedSingle IntersectTriangleWithPlane(const Triangle& triangle, const Plane& plane);

Segment IntersectSegmentWithPlane(const Segment& segment, const Plane& plane);

struct ScreenPoint {
  inline bool operator==(const ScreenPoint& left) const = default;

  inline V3 Float() const { return V3{float(x), float(y), z}; }

  int16_t x;
  int16_t y;
  float z;

  TextureCoordinates texture_coordinates;
};

struct ScreenSegment {
  ScreenPoint a;
  ScreenPoint b;
};

enum LineStatus { Vertical, NonVertical };

inline LineStatus GetLineStatus(const ScreenPoint& first, const ScreenPoint& second) {
  if (first.x == second.x) {
    return LineStatus::Vertical;
  }
  return LineStatus::NonVertical;
}

float GetTangentCoefficent(const ScreenPoint& first, const ScreenPoint& second);

struct ScreenTriangle {
  ScreenTriangle SortedVertex() const;
  int32_t MinimumHeight() const;
  int32_t MaximumHeight() const;

  ScreenPoint a;
  ScreenPoint b;
  ScreenPoint c;
  uint8_t model_index = 255;
};

ScreenSegment DiscretizeSegment(const Segment& segment);

ScreenTriangle DiscretizeTriangle(const Triangle& triangle);

M4 GetFrustumMatrix(
    HorizontalFOV horizontal_fov, AspectRatio aspect_ratio, NearPlaneDistance near_plane_distance,
    RenderDistance render_distance, RightEdgeX r, LeftEdgeX l, TopEdgeY t, BottomEdgeY b
);

struct BarycentricCoordinates {
  float a_coef;
  float b_coef;
  float c_coef;
};

inline BarycentricCoordinates
GetBarycentricCoordinates(const V3& a, const V3& b, const V3& c, const Point& point) {
  assert(false);
  V3 vector = {point.X(), point.Y(), point.Z()};
  float area = glm::length(glm::cross(a - b, a - c));
  float c_coef = glm::length(glm::cross(vector - a, vector - b)) / area;
  float a_coef = glm::length(glm::cross(vector - b, vector - c)) / area;
  float b_coef = glm::length(glm::cross(vector - a, vector - c)) / area;

  return {a_coef, b_coef, c_coef};
};

namespace {
inline int32_t FindDoubledSquare(const ScreenPoint& a, const ScreenPoint& b, const ScreenPoint& c) {
  return abs(a.x * b.y + b.x * c.y + c.x * a.y - a.x * c.y - b.x * a.y - c.x * b.y);
}
} // namespace

inline BarycentricCoordinates
GetBarycentricCoordinates(const ScreenTriangle& triangle, const ScreenPoint& point) {
  float area = FindDoubledSquare(triangle.a, triangle.b, triangle.c);
  float a_coef = float(FindDoubledSquare(triangle.b, triangle.c, point)) / area;

  float b_coef = float(FindDoubledSquare(triangle.a, triangle.c, point)) / area;
  return {a_coef, b_coef, 1.0f - a_coef - b_coef};
}

inline float InterpolateZ(const ScreenTriangle& screen_triangle, const ScreenPoint& screen_point) {
  BarycentricCoordinates bar_coords = GetBarycentricCoordinates(screen_triangle, screen_point);

  float z_inv = (1.0f / screen_triangle.a.z) * bar_coords.a_coef +
                (1.0f / screen_triangle.b.z) * bar_coords.b_coef +
                (1.0f / screen_triangle.c.z) * bar_coords.c_coef;
  return 1.0f / z_inv;
}

inline TextureCoordinates InterpolateTextureCoordinates(
    const ScreenTriangle& screen_triangle, const ScreenPoint& screen_point
) {
  BarycentricCoordinates bar_coords = GetBarycentricCoordinates(screen_triangle, screen_point);

  TextureCoordinates texture_coordinates =
      (screen_triangle.a.texture_coordinates * (1.0f / screen_triangle.a.z) * bar_coords.a_coef +
       screen_triangle.b.texture_coordinates * (1.0f / screen_triangle.b.z) * bar_coords.b_coef +
       screen_triangle.c.texture_coordinates * (1.0f / screen_triangle.c.z) * bar_coords.c_coef) *
      screen_point.z;

  texture_coordinates.u = std::max(texture_coordinates.u, 0.f);
  texture_coordinates.u = std::min(1.f, texture_coordinates.u);

  texture_coordinates.v = std::max(texture_coordinates.v, 0.f);
  texture_coordinates.v = std::min(1.f, texture_coordinates.v);
  return texture_coordinates;
}

} // namespace geometry

} // namespace detail
