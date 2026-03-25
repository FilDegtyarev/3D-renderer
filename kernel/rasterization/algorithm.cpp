#include "rasterization/algorithm.h"

#include "geometry/geometry.h"

#include <qnamespace.h>

namespace detail {

namespace rasterization {

std::vector<ScreenPoint> Bresenham(const ScreenPoint& start, const ScreenPoint& finish) {
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

  ScreenPoint current = start;
  std::vector<ScreenPoint> rasterized_segment;

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
static float EPS = 1e-9;
using LineStatus = geometry::LineStatus;
float GetXShift(const ScreenPoint& first, const ScreenPoint& second) {
  float x_shift = 0;
  if (geometry::GetLineStatus(first, second) == LineStatus::NonVertical) {
    x_shift = geometry::GetTangentCoefficent(first, second);
  }
  return x_shift;
}

bool InTriangle(float x) {
  if (x - ((int32_t)(x)) - 0.5 < -EPS) {
    return false;
  }
  return true;
}

} // namespace

void Scanline(
    const ScreenTriangle& triangle, int32_t height, std::vector<ScreenPoint>& scanline_buffer
) {
  // Shirley, 166
  ScreenTriangle sorted_triangle = triangle.SortedVertex();

  assert(sorted_triangle.a.y >= height && sorted_triangle.c.y <= height);

  if (sorted_triangle.a.y == sorted_triangle.c.y) {
    return;
  }

  float long_edge_x_shift = GetXShift(sorted_triangle.a, sorted_triangle.c);
  float x_left = 0;
  float x_right = 0;

  if (height >= sorted_triangle.b.y) {
    if (height == sorted_triangle.b.y && sorted_triangle.b.y == sorted_triangle.a.y) {
      x_left = sorted_triangle.b.x;
      x_right = sorted_triangle.a.x;
    } else {
      float short_edge_x_shift = GetXShift(sorted_triangle.a, sorted_triangle.b);
      float dy = sorted_triangle.a.y - height;

      x_left = float(sorted_triangle.a.x) - dy * long_edge_x_shift;
      x_right = float(sorted_triangle.a.x) - dy * short_edge_x_shift;
    }
    if (x_left > x_right) {
      std::swap(x_left, x_right);
    }
  } else if (height == sorted_triangle.c.y) {
    x_left = float(sorted_triangle.c.x);
    x_right = float(sorted_triangle.c.x);
  } else {
    if (height == sorted_triangle.b.y && sorted_triangle.b.y == sorted_triangle.c.y) {
      x_left = sorted_triangle.b.x;
      x_right = sorted_triangle.c.x;
    } else {
      float short_edge_x_shift = GetXShift(sorted_triangle.b, sorted_triangle.c);
      float dy = sorted_triangle.b.y - height;
      x_left = float(sorted_triangle.a.x) - (sorted_triangle.a.y - height) * long_edge_x_shift;
      x_right = float(sorted_triangle.b.x) - dy * short_edge_x_shift;
    }

    if (x_left > x_right) {
      std::swap(x_left, x_right);
    }
  }

  int32_t start = int32_t(x_left);
  int32_t finish = int32_t(x_right);
  start = std::ceil(x_left);
  finish = std::ceil(x_right) - 1;

  // int32_t start = std::ceil(x_left - 0.5f);
  // int32_t finish = std::floor(x_right - 0.5f);

  for (int32_t x = start; x <= finish; ++x) {
    ScreenPoint point;
    point.x = x;
    point.y = height;
    scanline_buffer.push_back(point);
  }

  for (ScreenPoint& screen_point : scanline_buffer) {
    screen_point.z = InterpolateZ(triangle, screen_point);
    screen_point.texture_coordinates = InterpolateTextureCoordinates(triangle, screen_point);
  }
}

} // namespace rasterization
} // namespace detail
