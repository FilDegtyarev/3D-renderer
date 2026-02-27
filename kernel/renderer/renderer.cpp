#include "renderer/renderer.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include <cstddef>
#include <memory>
namespace detail {

namespace renderer {

Renderer::Renderer(int32_t total_workers, std::vector<QRgb> &flat_screen,
                   concurrency::WorkerKeeper &&worker_keeper_)
    : total_workers(total_workers), worker_keeper(std::move(worker_keeper_)) {

  zbuffer = ZBuffer(worker_keeper.screen_height, std::vector<ZColor>(worker_keeper_.screen_width));
  segment_rasterizer = rasterization::DrawSegment;
  worker_keeper.UnleashWorkers(flat_screen, zbuffer);
}

void Renderer::Render() {
  // std::cout << "rendering\n\n\n\n" << std::endl;
  M4 frsutum_matrix = worker_keeper.camera.GetFrustumMatrix();
  M4 camera_matrix = worker_keeper.camera.GetCameraMatrix();
  // ClearZBuffer();
  //  std::cout << "уже неплохо" << std::endl;
  //
  //  std::cout << "[master]: wait for clear" << std::endl;
  worker_keeper.WaitForClear();
  // Начинается очистка

  // std::cout << "[master]: wait for clip" << std::endl;
  worker_keeper.WaitForClip();
  // Начинается клиппинг

  // std::cout << "[master]: wait for draw" << std::endl;
  worker_keeper.WaitForDraw();
  // Начинается рисование
  // Остался последний этап
  // std::cout << "Done" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(16));
}
// int32_t screen_height = worker_keeper.screen_height;
// std::vector<std::vector<Color>> screen(worker_keeper.screen_height,
//                                        std::vector<Color>(worker_keeper.screen_width));

// for (size_t i = 0; i < worker_keeper.screen_height; ++i) {
//   for (size_t j = 0; j < worker_keeper.screen_width; ++j) {
//     screen[i][j] = zbuffer[i][j].color;
//     // std::cout << "master done\n" << std::endl;
//   }
// }
// std::cout << "master says: " << flag << std::endl;
//  std::cout << "master done\n" << std::endl;

void Renderer::FrameSucceed() { worker_keeper.WaitForClear(); }

ZBuffer &Renderer::GetZBuffer() { return zbuffer; }

void Renderer::ClearZBuffer() {
  for (int i = 0; i < zbuffer.size(); ++i) {
    for (int j = 0; j < zbuffer[i].size(); ++j) {
      zbuffer[i][j].color = {0, 0, 0};
      zbuffer[i][j].z = DBL_MAX;
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

    segment_rasterizer(projective_segment, zbuffer);
  }
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle_, const M4 &frustum_matrix,
                              const M4 &camera_matrix, const camera::Camera &camera) {
  const geometry::Triangle triangle = triangle_ * camera_matrix;
  for (const geometry::Triangle &clipped_triangle : camera.ClipTriangle(triangle)) {

    geometry::Point a_proj =
        FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.a),
               clipped_triangle.a.color);

    geometry::Point b_proj =
        FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.b),
               clipped_triangle.b.color);

    geometry::Point c_proj =
        FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.c),
               clipped_triangle.c.color);

    geometry::Triangle projective_triangle =
        geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};
    ViewTriangleTransform(projective_triangle);
    triangle_rasterizer(projective_triangle, zbuffer);
  }
}

} // namespace renderer
} // namespace detail
