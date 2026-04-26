#include "rasterization/rasterization.h"

#include "geometry/geometry.h"
#include "glm/geometric.hpp"
#include "rasterization/algorithm.h"
#include "shadows/shadow_map.h"
#include "types/types.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
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

bool IsInShadowMap(const ScreenPoint& point, const ShadowMap& shadow_map) {
  return !(
      point.x < 0 || point.x >= shadow_map.GetWidth() || point.y < 0 ||
      point.y >= shadow_map.GetHeight()
  );
}
} // namespace

void DrawSegment(const Segment& segment_, ZBuffer& zbuffer) {
  assert(false);
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

      // zbuffer(pixel.y, pixel.x).color = pixel.color;
      // zbuffer(pixel.y, pixel.x).color = Color{.red = 50, .blue = 100, .green = 150};
    }
  }
}

namespace {
inline float InterpolateW(
    const ScreenTriangle& triangle, const ShadowMapHelper* helper, const ScreenPoint& point
) {
  geometry::BarycentricCoordinates bc = geometry::GetBarycentricCoordinates(triangle, point);
  return helper->a_w * bc.a_coef + helper->b_w * bc.b_coef + helper->c_w * bc.c_coef;
}

inline float PerspectiveInterpolateW(
    const ScreenTriangle& triangle, const ShadowMapHelper* helper, const ScreenPoint& screen_point
) {
  geometry::BarycentricCoordinates bar_coords = GetBarycentricCoordinates(triangle, screen_point);

  float w_inv =
      ((1.0f / helper->a_w) * bar_coords.a_coef + (1.0f / helper->b_w) * bar_coords.b_coef +
       (1.0f / helper->c_w) * bar_coords.c_coef);
  return 1.0f / w_inv;
}

} // namespace

void DrawTriangle(
    const Triangle& triangle, ZBuffer& zbuffer, std::vector<ScreenPoint>& scanline_buffer,
    const Texture& texture, const DirectionalLightSource* direction_light,
    const ShadowMapHelper* helper, const Triangle& world_triangle
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

      if (pixel.z < zbuffer(pixel.y, pixel.x).z) {
        Color color = {255, 255, 255};
        //{uint8_t(rand() % 256), uint8_t(rand() % 256), uint8_t(rand() % 256)};

        if (texture.IsActive()) {
          color = texture(pixel.texture_coordinates.u, pixel.texture_coordinates.v);
        }

        geometry::BarycentricCoordinates bc = GetBarycentricCoordinates(screen_triangle, pixel);

        V3 interpolated_normal = glm::normalize(
            (triangle.a.normal * (1.0f / screen_triangle.a.z) * bc.a_coef +
             triangle.b.normal * (1.0f / screen_triangle.b.z) * bc.b_coef +
             triangle.c.normal * (1.0f / screen_triangle.c.z) * bc.c_coef) *
            pixel.z
        );

        color = direction_light->CalculateColor(interpolated_normal, color);

        // color.red = uint8_t((interpolated_normal.x * 0.5f + 0.5f) * 255);
        // color.green = uint8_t((interpolated_normal.y * 0.5f + 0.5f) * 255);
        // color.blue = uint8_t((interpolated_normal.z * 0.5f + 0.5f) * 255);

        if (helper != nullptr) {
          V4 origin = {float(pixel.x), float(pixel.y), pixel.z, 1};
          float correct_w = InterpolateW(screen_triangle, helper, pixel);
          origin = *helper->view_transform_inv * origin;
          origin.w = 1;
          origin *= correct_w;
          origin = *helper->frustum_to_world * origin;
          origin = *helper->world_to_light * origin;

          color = shadow_mapping::ShadowTest(Point{origin}, color, helper->shadow_map);
        }

        zbuffer(pixel.y, pixel.x).z = pixel.z;
        zbuffer(pixel.y, pixel.x).color = color;
      }
    }
  }
}

void DrawTriangleInShadowMap(
    const Triangle& triangle, ShadowMap& shadow_map, std::vector<ScreenPoint>& scanline_buffer
) {

  ScreenTriangle screen_triangle = geometry::DiscretizeTriangle(triangle);
  for (size_t height = screen_triangle.MinimumHeight(); height <= screen_triangle.MaximumHeight();
       ++height) {
    scanline_buffer.clear();

    // По хоршему стоит наверное добавить флаги, типа надо ли интерполировать. Но это потом))
    ShadowMapScanline(screen_triangle, height, scanline_buffer);

    for (auto& pixel : scanline_buffer) {

      if (!IsInShadowMap(pixel, shadow_map)) {
        continue;
      }
      shadow_map(pixel.y, pixel.x) = std::min(shadow_map(pixel.y, pixel.x), 1 + pixel.z);
    }
  }
}

} // namespace rasterization
} // namespace detail
