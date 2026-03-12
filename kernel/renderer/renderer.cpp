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

} // namespace renderer
} // namespace detail
