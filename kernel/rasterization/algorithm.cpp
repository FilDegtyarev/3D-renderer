#include "algorithm.h"
#include "geometry/geometry.h"
#include <iostream>
#include <qnamespace.h>
namespace detail {

namespace rasterization {

std::vector<geometry::ScreenPoint>
Bresenham(const geometry::ScreenPoint &start,
          const geometry::ScreenPoint &finish) {
  if (start.x > finish.x) {
    return Bresenham(finish, start);
  }

  assert(start.x <= finish.x);

  int32_t dx = std::abs(start.x - finish.x);
  int32_t dy = std::abs(start.y - finish.y);

  int32_t sgn_y = (start.y <= finish.y ? 1 : -1);

  int32_t right_shift = 2 * dy;
  int32_t diagonal_shift = 2 * (dy - dx);
  int32_t d = 2 * dy - dx;

  geometry::ScreenPoint current = start;
  std::vector<geometry::ScreenPoint> rasterized_segment;

  if (dx >= dy) {
    for (size_t i = 0; i < dx; ++i) {
      rasterized_segment.push_back(current);
      if (d > 0) {
        current.y += sgn_y;
        d += diagonal_shift;
      } else {
        d += right_shift;
      }

      current.x++;
    }

  } else {
    // Прямая более горизонтальна
    d = 2 * dx - dy;
    right_shift = 2 * dx;
    diagonal_shift = 2 * (dx - dy);

    for (size_t i = 0; i < dy; ++i) {
      rasterized_segment.push_back(current);
      if (d > 0) {
        d += diagonal_shift;
        current.x++;
      } else {
        d += right_shift;
      }

      current.y += sgn_y;
    }
  }

  rasterized_segment.push_back(finish);
  return rasterized_segment;
}

namespace {
const double EPS = 1e-9;
double GetXShift(const geometry::ScreenPoint &first,
                 const geometry::ScreenPoint &second) {
  double x_shift = 0;
  if (geometry::GetLineStatus(first, second) ==
      geometry::LineStatus::NonVertical) {
    x_shift = geometry::GetTangentCoefficent(first, second);
  }
  return x_shift;
}

bool InTriangle(double x) {
  if (x - ((int32_t)(x)) - 0.5 < -EPS) {
    return false;
  }
  return true;
}

} // namespace

std::vector<geometry::ScreenPoint>
Scanline(const geometry::ScreenTriangle &triangle, int32_t height) {
  // Shirley, 166
  geometry::ScreenTriangle sorted_triangle = triangle.SortedVertex();
  assert(sorted_triangle.a.y >= height && sorted_triangle.c.y <= height);

  double long_edge_x_shift = GetXShift(sorted_triangle.a, sorted_triangle.c);
  double x_left = 0;
  double x_right = 0;

  if (height >= sorted_triangle.b.y) {
    double short_edge_x_shift = GetXShift(sorted_triangle.a, sorted_triangle.b);
    double dy = sorted_triangle.a.y - height;

    x_left = double(sorted_triangle.a.x) - dy * long_edge_x_shift;
    x_right = double(sorted_triangle.a.x) - dy * short_edge_x_shift;

    if (x_left > x_right) {
      std::swap(x_left, x_right);
    }

  } else {
    double short_edge_x_shift = GetXShift(sorted_triangle.b, sorted_triangle.c);
    double dy = sorted_triangle.b.y - height;
    x_left = double(sorted_triangle.a.x) -
             (sorted_triangle.a.y - height) * long_edge_x_shift;
    x_right = double(sorted_triangle.b.x) - dy * short_edge_x_shift;

    if (x_left > x_right) {
      std::swap(x_left, x_right);
    }
  }

  int32_t start;
  int32_t finish;
  if (!InTriangle(x_left)) {
    if (start != INT32_MAX) {
      start = int32_t(x_left) + 1;
    }
  } else {
    start = int32_t(x_left);
  }

  if (!InTriangle(x_right)) {
    if (finish != 0) {
      finish = int32_t(x_right) - 1;
    }
  } else {
    finish = int32_t(x_right);
  }

  std::vector<geometry::ScreenPoint> segment;
  for (size_t x = start; x <= finish; ++x) {
    geometry::ScreenPoint point;
    point.x = x;
    point.y = height;

    point.color = Color{.red = 255, .green = 51, .blue = 153};
    segment.push_back(point);
  }

  return segment;
}

} // namespace rasterization
} // namespace detail
