#pragma once
#include "geometry/geometry.h"
#include "types/types.h"

namespace detail {
namespace rasterization {
void DrawSegment(const geometry::Segment &segment, ZBuffer &zbuffer);
void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer);

} // namespace rasterization

} // namespace detail
