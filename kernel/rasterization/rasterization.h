#pragma once
#include "geometry/geometry.h"
#include "textures/textures.h"
#include "types/types.h"
#include <vector>

namespace detail {
namespace rasterization {
void DrawSegment(const geometry::Segment &segment, ZBuffer &zbuffer);
void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer,
                  std::vector<geometry::ScreenPoint> &scanline_buffer);
void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer,
                  std::vector<geometry::ScreenPoint> &scanline_buffer,
                  const textures::Texture &texture);

} // namespace rasterization

} // namespace detail
