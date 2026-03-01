#include "renderer/renderer.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include "world/world.h"
#include <cstddef>
#include <memory>
namespace detail {

namespace renderer {

Renderer::Renderer(int32_t total_workers, std::vector<QRgb> &flat_screen,
                   concurrency::WorkerKeeper &&worker_keeper_)
    : total_workers(total_workers), worker_keeper(std::move(worker_keeper_)) {
  zbuffer = ZBuffer(worker_keeper.screen_height, worker_keeper.screen_width);
  worker_keeper.UnleashWorkers(flat_screen, zbuffer);
  // triangle_rasterizer = rasterization::DrawTriangle(const geometry::Triangle &triangle, ZBuffer
  // &zbuffer, std::vector<geometry::ScreenPoint> &scanline_buffer)
}

void Renderer::Render() {
  // std::cout << "rendering\n\n\n\n" << std::endl;
  // M4 frustum_matrix = worker_keeper.camera.GetFrustumMatrix();
  // M4 camera_matrix = worker_keeper.camera.GetCameraMatrix();
  // ClearZBuffer();
  worker_keeper.WaitForClear();
  worker_keeper.WaitForClip();
  worker_keeper.WaitForDraw();
  // for (const world::GlobalObject &object : worker_keeper.world.GetObjects()) {
  //   RenderGlobalObject(object, frustum_matrix, camera_matrix, worker_keeper.camera);
  // }
}

void Renderer::FrameSucceed() { worker_keeper.WaitForClear(); }

ZBuffer &Renderer::GetZBuffer() { return zbuffer; }

void Renderer::ClearZBuffer() {
  for (int i = 0; i < zbuffer.GetHeight(); ++i) {
    for (int j = 0; j < zbuffer.GetWidth(); ++j) {
      zbuffer.At(i, j).color = {0, 0, 0};
      zbuffer.At(i, j).z = FLT_MAX;
    }
  }
}
void Renderer::RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum_matrix,
                                  const M4 &camera_matrix, const camera::Camera &camera) {
  int i = 0;
  for (const geometry::Triangle &triangle : object.GetTriangles()) {
    i++;
    RenderTriangle(triangle, frustum_matrix, camera_matrix, camera);
  }

  for (const geometry::Segment &segment : object.GetSegments()) {
    RenderSegment(segment, frustum_matrix, camera_matrix, camera);
  }
}

namespace {
inline geometry::Point FromV4(const V4 &vector, Color color) {
  return geometry::Point{
      .x = vector.x / vector.w, .y = vector.y / vector.w, .z = vector.z / vector.w, .color = color};
}
} // namespace

void Renderer::RenderSegment(const geometry::Segment &segment_, const M4 &frustum_matrix,
                             const M4 &camera_matrix, const camera::Camera &camera) {
  geometry::Segment segment = segment_ * camera_matrix;
  for (const geometry::Segment &clipped_segment : camera.ClipSegment(segment)) {
    geometry::Point a_proj = FromV4(
        frustum_matrix * geometry::SwitchToProjective(clipped_segment.a), clipped_segment.a.color);

    geometry::Point b_proj = FromV4(
        frustum_matrix * geometry::SwitchToProjective(clipped_segment.b), clipped_segment.b.color);

    geometry::Segment projective_segment = geometry::Segment{a_proj, b_proj};

    ViewSegmentTransform(projective_segment);

    // segment_rasterizer(projective_segment, zbuffer);
  }
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle_, const M4 &frustum_matrix,
                              const M4 &camera_matrix, const camera::Camera &camera) {
  const geometry::Triangle triangle = triangle_ * camera_matrix;
  // for (const geometry::Triangle &clipped_triangle : worker_keeper.camera.ClipTriangle(triangle))
  // {

  //   geometry::Point a_proj =
  //       FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.a),
  //              clipped_triangle.a.color);

  //   geometry::Point b_proj =
  //       FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.b),
  //              clipped_triangle.b.color);

  //   geometry::Point c_proj =
  //       FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.c),
  //              clipped_triangle.c.color);

  //   geometry::Triangle projective_triangle =
  //       geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};
  //   ViewTriangleTransform(projective_triangle);
  //   auto v = std::vector<geometry::ScreenPoint>(screen_width + screen_height);
  //   rasterization::DrawTriangle(projective_triangle, zbuffer, v);
  // }
}

} // namespace renderer
} // namespace detail
