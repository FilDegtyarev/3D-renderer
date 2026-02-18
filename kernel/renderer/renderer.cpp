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

std::vector<std::vector<Color>> Renderer::Render(const std::unique_ptr<world::World> &world, const std::unique_ptr<camera::Camera> &camera) {
  M4 frsutum_matrix = camera->GetFrustumMatrix();
  M4 camera_matrix = camera->GetCameraMatrix();
  ClearZBuffer();

  for (const world::GlobalObject &object : world->GetObjects()) {
    RenderGlobalObject(object, frsutum_matrix, camera_matrix, camera);
  }

  std::vector<std::vector<Color>> screen(screen_height, std::vector<Color>(screen_width));
  for (size_t i = 0; i < screen_height; ++i) {
    for (size_t j = 0; j < screen_width; ++j) {
      screen[i][j] = zbuffer[i][j].color;
    }
  }
  return screen;
}

void Renderer::ClearZBuffer() {
  for (int i = 0; i < zbuffer.size(); ++i) {
    for (int j = 0; j < zbuffer[i].size(); ++j) {
      zbuffer[i][j].color = {0, 0, 0};
      zbuffer[i][j].z = DBL_MAX;
    }
  }
}
void Renderer::RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum_matrix, const M4 &camera_matrix, const std::unique_ptr<camera::Camera> &camera) {
  int i = 0;
  for (const geometry::Triangle &triangle : object.GetTriangles()) {
    RenderTriangle(triangle, frustum_matrix, camera_matrix);
  }

  for (const geometry::Segment &segment : object.GetSegments()) {
    RenderSegment(segment, frustum_matrix, camera_matrix, camera);
  }
}

namespace {
inline geometry::Point FromV4(const V4 &vector, Color color) { return geometry::Point{.x = vector.x / vector.w, .y = vector.y / vector.w, .z = vector.z / vector.w, .color = color}; }
} // namespace

void Renderer::RenderSegment(const geometry::Segment &segment_, const M4 &frustum_matrix, const M4 &camera_matrix, const std::unique_ptr<camera::Camera> &camera) {
  geometry::Segment segment = segment_ * camera_matrix;
  for (const geometry::Segment &clipped_segment : camera->ClipSegment(segment)) {
    geometry::Point a_proj = FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_segment.a), clipped_segment.a.color);

    geometry::Point b_proj = FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_segment.b), clipped_segment.b.color);

    geometry::Segment projective_segment = geometry::Segment{a_proj, b_proj};

    ViewSegmentTransform(projective_segment);

    segment_rasterizer(projective_segment, zbuffer);
  }
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle_, const M4 &frustum_matrix, const M4 &camera_matrix) {
  geometry::Triangle triangle = triangle_ * camera_matrix;
  assert(false);
  geometry::Point a_proj = FromV4(frustum_matrix * geometry::SwitchToProjective(triangle.a), triangle.a.color);

  geometry::Point b_proj = FromV4(frustum_matrix * geometry::SwitchToProjective(triangle.b), triangle.b.color);

  geometry::Point c_proj = FromV4(frustum_matrix * geometry::SwitchToProjective(triangle.c), triangle.c.color);

  geometry::Triangle projective_triangle = geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};

  ViewTriangleTransform(projective_triangle);
  triangle_rasterizer(projective_triangle, zbuffer);
}

} // namespace renderer
} // namespace detail
