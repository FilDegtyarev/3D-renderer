#include "rasterization/rasterization.h"

#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "types/types.h"

#include <cassert>
#include <cstddef>
#include <qpoint.h>
#include <qrgb.h>

namespace detail {

namespace rasterization {

namespace {
bool IsInBuffer(const ScreenPoint& point, const ZBuffer& zbuffer) {
  return !(
      point.x < 0 || point.x >= zbuffer.GetWidth() || point.y < 0 || point.y >= zbuffer.GetHeight()
  );
}

} // namespace

void DrawSegment(const Segment& segment_, ZBuffer& zbuffer) {
  ScreenSegment segment = geometry::DiscretizeSegment(segment_);
  ScreenPoint from = segment.a;
  ScreenPoint to = segment.b;

  std::vector<ScreenPoint> line = rasterization::Bresenham(from, to);

  for (const auto& pixel : line) {
    if (!IsInBuffer(pixel, zbuffer)) {
      continue;
    }

    if (zbuffer(pixel.y, pixel.x).z > pixel.z) {
      zbuffer(pixel.y, pixel.x).z = pixel.z;

      zbuffer(pixel.y, pixel.x).color = pixel.color;
      zbuffer(pixel.y, pixel.x).color = Color{.red = 50, .blue = 100, .green = 150};
    }
  }
}

void DrawTriangle(
    const Triangle& triangle, ZBuffer& zbuffer, std::vector<ScreenPoint>& scanline_buffer,
    const Texture& texture
) {

  ScreenTriangle screen_triangle = geometry::DiscretizeTriangle(triangle);

  for (size_t height = screen_triangle.MinimumHeight(); height <= screen_triangle.MaximumHeight();
       ++height) {
    scanline_buffer.clear();
    Scanline(screen_triangle, height, scanline_buffer);

    for (auto& pixel : scanline_buffer) {
      if (!IsInBuffer(pixel, zbuffer)) {
        continue;
      }

      if (texture.IsActive()) {
        pixel.color = texture(pixel.texture_coordinates.u, pixel.texture_coordinates.v);
      }

      if (pixel.z < zbuffer(pixel.y, pixel.x).z) {
        zbuffer(pixel.y, pixel.x).z = pixel.z;
        zbuffer(pixel.y, pixel.x).color = pixel.color;
      }
    }
  }
}

} // namespace rasterization
} // namespace detail
