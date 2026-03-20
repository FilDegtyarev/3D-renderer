#pragma once
#include "types/types.h"

namespace detail {
namespace geometry {
struct Point {
  Point &operator=(const Point &point) = default;

  Point operator+(const V3 &vector) const;
  Point operator+=(const V3 &vector);

  void Scale(const float &coef);
  Point operator*(const M4 &matrix) const;

  bool operator==(const Point &other) const = default;

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
  Color color;
  TextureCoordinates texture_coordinates;
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
  float D;

  float operator()(const Point &point) const;
};

struct TriangleIntersectedSingle {
  void Append(const Triangle &triangle);
  Triangle triangles[2];
  int32_t size;
};

struct TriangleIntersected {
  void Merge(const TriangleIntersectedSingle &single);
  void Clear();

  const Triangle &operator[](size_t i);
  Triangle triangles[64];
  int32_t size;
};

TriangleIntersectedSingle IntersectTriangleWithPlane(const Triangle &triangle, const Plane &plane);

// Какой отрезок получится, если пересечь с плоскостью?
Segment IntersectSegmentWithPlane(const Segment &segment, const Plane &plane);

V4 SwitchToProjective(const Point &point);

struct ScreenPoint {
  inline bool operator==(const ScreenPoint &left) const = default;

  inline V3 Float() const { return V3{x, y, z}; }

  int32_t x;
  int32_t y;
  float z;

  Color color;
  TextureCoordinates texture_coordinates;
};

struct ScreenSegment {
  ScreenPoint a;
  ScreenPoint b;
};

enum LineStatus { Vertical, NonVertical };

LineStatus GetLineStatus(const ScreenPoint &first, const ScreenPoint &second);

float GetTangentCoefficent(const ScreenPoint &first, const ScreenPoint &second);

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

struct BarycentricCoordinates {
  float a_coef;
  float b_coef;
  float c_coef;
};

inline BarycentricCoordinates GetBarycentricCoordinates(const V3 &a, const V3 &b, const V3 &c,
                                                        const Point &point) {
  V3 vector = {point.X(), point.Y(), point.Z()};
  float ab_coef = abs(glm::dot(vector - a, vector - b));
  float bc_coef = abs(glm::dot(vector - b, vector - c));
  float ac_coef = abs(glm::dot(vector - a, vector - c));

  return {ab_coef, bc_coef, ac_coef};
};

inline BarycentricCoordinates GetBarycentricCoordinates(const V3 &a, const V3 &b, const V3 &c,
                                                        const ScreenPoint &point) {
  V3 vector = point.Float();
  float ab_coef = abs(glm::dot(vector - a, vector - b));
  float bc_coef = abs(glm::dot(vector - b, vector - c));
  float ac_coef = abs(glm::dot(vector - a, vector - c));

  return {ab_coef, bc_coef, ac_coef};
}

// Не оптимизировано
inline float InterpolateZ(const ScreenTriangle &screen_triangle, const ScreenPoint &screen_point) {
  V3 a = screen_triangle.a.Float();
  V3 b = screen_triangle.b.Float();
  V3 c = screen_triangle.c.Float();

  BarycentricCoordinates bar_coords = GetBarycentricCoordinates(a, b, c, screen_point);

  float z_inv = 1.0f / screen_triangle.a.z * bar_coords.a_coef +
                1.0f / screen_triangle.b.z * bar_coords.b_coef +
                1.0f / screen_triangle.c.z * bar_coords.c_coef;
  return 1.0f / z_inv;
}

inline TextureCoordinates InterpolateTextureCoordinates(const ScreenTriangle &screen_triangle,
                                                        const ScreenPoint &screen_point) {
  V3 a = screen_triangle.a.Float();
  V3 b = screen_triangle.b.Float();
  V3 c = screen_triangle.c.Float();
  BarycentricCoordinates bar_coords = GetBarycentricCoordinates(a, b, c, screen_point);

  //
  TextureCoordinates texture_coordinates =
      (screen_triangle.a.texture_coordinates * (1.0f / screen_triangle.a.z) * bar_coords.a_coef +
       screen_triangle.b.texture_coordinates * (1.0f / screen_triangle.b.z) * bar_coords.b_coef +
       screen_triangle.c.texture_coordinates * (1.0f / screen_triangle.c.z) * bar_coords.c_coef) *
      screen_point.z;
  return texture_coordinates;
}

} // namespace geometry

} // namespace detail
