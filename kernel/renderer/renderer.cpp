#include "renderer.h"
#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include <cstddef>
namespace detail {

namespace renderer {

Renderer::Renderer(ScreenHeight _screen_height, ScreenWidth _screen_width) {
  screen_height = _screen_height();
  screen_width = _screen_width();

  zbuffer = ZBuffer(screen_height, std::vector<ZColor>(screen_width));

  segment_rasterizer = rasterization::DrawSegment;
  triangle_rasterizer = rasterization::DrawTriangle;
}

std::vector<std::vector<Color>> Renderer::Render(const world::World &world,
                                                 const camera::Camera &camera) {
  M4 frsutum = camera.GetFrustumMatrix();

  for (const world::GlobalObject &object : world.GetObjects()) {
    RenderGlobalObject(object, frsutum);
  }

  std::vector<std::vector<Color>> screen(screen_height,
                                         std::vector<Color>(screen_width));
  for (size_t i = 0; i < screen_height; ++i) {
    for (size_t j = 0; j < screen_width; ++j) {
      screen[i][j] = zbuffer[i][j].color;
    }
  }
  return screen;
}

void Renderer::RenderGlobalObject(const world::GlobalObject &object,
                                  const M4 &frustum) {
  int i = 0;
  for (const geometry::Triangle &triangle : object.GetTriangles()) {
    RenderTriangle(triangle, frustum);
  }

  for (const geometry::Segment &segment : object.GetSegments()) {
    RenderSegment(segment, frustum);
  }
}

namespace {
inline geometry::Point FromV4(const V4 &vector, Color color) {
  return geometry::Point{.x = vector.x / vector.w,
                         .y = vector.y / vector.w,
                         .z = vector.z / vector.w,
                         .color = color};
}
} // namespace

void Renderer::RenderSegment(const geometry::Segment &segment,
                             const M4 &frustum) {
  geometry::Point a_proj = FromV4(
      frustum * geometry::SwitchToProjective(segment.a), segment.a.color);

  geometry::Point b_proj = FromV4(
      frustum * geometry::SwitchToProjective(segment.b), segment.b.color);

  geometry::Segment projective_segment = geometry::Segment{a_proj, b_proj};

  ViewSegmentTransform(projective_segment);

  segment_rasterizer(projective_segment, zbuffer);
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle,
                              const M4 &frustum) {

  geometry::Point a_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.a), triangle.a.color);

  geometry::Point b_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.b), triangle.b.color);

  geometry::Point c_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.c), triangle.c.color);

  geometry::Triangle projective_triangle =
      geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};

  ViewTriangleTransform(projective_triangle);
  triangle_rasterizer(projective_triangle, zbuffer);
}

} // namespace renderer
} // namespace detail