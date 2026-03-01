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
  if (point.x < 0 || point.x >= zbuffer.GetWidth() || point.y < 0 ||
      point.y >= zbuffer.GetHeight()) {
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

    if (zbuffer.At(pixel.y, pixel.x).z > pixel.z) {
      zbuffer.At(pixel.y, pixel.x).z = pixel.z;

      zbuffer.At(pixel.y, pixel.x).color = pixel.color;
      zbuffer.At(pixel.y, pixel.x).color = Color{.red = 50, .blue = 100, .green = 150};
    }
  }
}

void DrawTriangle(const geometry::Triangle &triangle, ZBuffer &zbuffer,
                  std::vector<geometry::ScreenPoint> &scanline_buffer) {
  geometry::ScreenTriangle screen_triangle = geometry::DiscretizeTriangle(triangle);

  for (size_t height = screen_triangle.MinimumHeight(); height <= screen_triangle.MaximumHeight();
       ++height) {
    scanline_buffer.clear();
    // std::vector<geometry::ScreenPoint> scanline =
    rasterization::Scanline(screen_triangle, height, scanline_buffer);

    for (const auto &pixel : scanline_buffer) {
      if (!InBuffer(pixel, zbuffer)) {
        continue;
      }
      if (pixel.z < zbuffer.At(pixel.y, pixel.x).z) {
        zbuffer.At(pixel.y, pixel.x).z = pixel.z;
        zbuffer.At(pixel.y, pixel.x).color = {uint8_t(rand() % 256), uint8_t(rand() % 256),
                                              uint8_t(rand() % 256)};
      }
    }
  }
}

} // namespace rasterization
} // namespace detail
