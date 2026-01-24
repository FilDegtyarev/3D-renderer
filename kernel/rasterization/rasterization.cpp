#include "rasterization.h"
namespace detail {

namespace rasterization {

std::vector<geometry::Point> SimpleFloat(const geometry::Point &start,
                                         const geometry::Point &finish) {
  return {};
}

std::vector<geometry::Point> Bresenham(const geometry::Point &start,
                                       const geometry::Point &finish) {
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

  geometry::Point current = start;
  std::vector<geometry::Point> rasterized_segment;

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

} // namespace rasterization
} // namespace detail
