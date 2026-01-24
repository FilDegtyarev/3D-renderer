#pragma once
#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace rasterization {
std::vector<geometry::Point> SimpleFloat(const geometry::Point &start,
                                         const geometry::Point &finish);

std::vector<geometry::Point> Bresenham(const geometry::Point &first,
                                       const geometry::Point &second);
} // namespace rasterization

} // namespace detail