#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "screen/screen.h"
#include "types/types.h"
#include <cassert>
#include <iostream>
#include <qpoint.h>
#include <qrgb.h>
namespace detail {

namespace rasterization {

const int32_t SIZE = 500;

namespace {
bool InBuffer(const geometry::ScreenPoint &point, const ZBuffer &zbuffer) {
  if (point.x < 0 || point.x >= zbuffer[0].size() || point.y < 0 || point.y >= zbuffer.size()) {
    return false;
  }
  return true;
}

} // namespace

void DrawSegment(const geometry::Segment &segment_, ZBuffer &zbuffer) {
  geometry::ScreenSegment segment = geometry::DiscretizeSegment(segment_);
  geometry::ScreenPoint from = segment.a;
  geometry::ScreenPoint to = segment.b;

  std::vector<detail::geometry::ScreenPoint> line = detail::rasterization::Bresenham(from, to);

  for (const auto &pixel : line) {
    if (!InBuffer(pixel, zbuffer)) {
      continue;
    }

    if (zbuffer.at(pixel.y).at(pixel.x).z > pixel.z) {
      zbuffer[pixel.y][pixel.x].z = pixel.z;

      zbuffer[pixel.y][pixel.x].color = pixel.color;
      zbuffer[pixel.y][pixel.x].color = Color{.red = 50, .blue = 100, .green = 150};
    }
  }
}

void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer, std::vector<geometry::ScreenPoint> &scanline_buffer) {
  assert(scanline_buffer.empty());
  geometry::ScreenTriangle screen_triangle = geometry::DiscretizeTriangle(triangle);

  for (size_t height = screen_triangle.MinimumHeight(); height <= screen_triangle.MaximumHeight(); ++height) {
    // std::vector<geometry::ScreenPoint> scanline =
    rasterization::Scanline(screen_triangle, height, scanline_buffer);

    for (const auto &pixel : scanline_buffer) {
      if (!InBuffer(pixel, zbuffer)) {
        continue;
      }
      if (pixel.z < zbuffer[pixel.y][pixel.x].z) {
        zbuffer[pixel.y][pixel.x].z = pixel.z;
        zbuffer[pixel.y][pixel.x].color = {uint8_t(rand() % 256), uint8_t(rand() % 256), uint8_t(rand() % 256)};
      }
    }
  }
}

} // namespace rasterization
} // namespace detail
