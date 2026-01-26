#pragma once
#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace rasterization {

std::vector<geometry::Point> Bresenham(const geometry::Point &first,
                                       const geometry::Point &second);

std::vector<geometry::Point> Scanline(const geometry::Triangle &triangle,
                                      int32_t height);
} // namespace rasterization

} // namespace detail