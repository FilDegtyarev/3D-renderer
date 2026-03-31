#pragma once
#include "geometry/geometry.h"
#include "types/types.h"

#include <vector>

namespace detail {
namespace rasterization {
using Point = geometry::Point;
using ScreenPoint = geometry::ScreenPoint;
using Triangle = geometry::Triangle;
using Segment = geometry::Segment;
using ScreenTriangle = geometry::ScreenTriangle;
using ScreenSegment = geometry::ScreenSegment;

std::vector<ScreenPoint> Bresenham(const ScreenPoint& first, const ScreenPoint& second);

void Scanline(
    const ScreenTriangle& triangle, int32_t height, std::vector<ScreenPoint>& scanline_buffer
);

void ShadowMapScanline(
    const ScreenTriangle& triangle, int32_t height, std::vector<ScreenPoint>& scanline_buffer
);
} // namespace rasterization

} // namespace detail
