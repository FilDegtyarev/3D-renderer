#pragma once
#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace rasterization {

std::vector<geometry::ScreenPoint>
Bresenham(const geometry::ScreenPoint &first,
          const geometry::ScreenPoint &second);

std::vector<geometry::ScreenPoint>
Scanline(const geometry::ScreenTriangle &triangle, int32_t height);
} // namespace rasterization

} // namespace detail