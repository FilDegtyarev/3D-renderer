#pragma once
#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace rasterization {

std::vector<geometry::ScreenPoint> Bresenham(const geometry::ScreenPoint &first, const geometry::ScreenPoint &second);

void Scanline(const geometry::ScreenTriangle &triangle, int32_t height, std::vector<geometry::ScreenPoint> &scanline_buffer);
} // namespace rasterization

} // namespace detail
