#pragma once
#include "geometry/geometry.h"
#include "types/types.h"
#include <vector>

namespace detail {
namespace rasterization {
void DrawSegment(const geometry::Segment &segment, ZBuffer &zbuffer);
void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer, std::vector<geometry::ScreenPoint> &scanline_buffer);

} // namespace rasterization

} // namespace detail
