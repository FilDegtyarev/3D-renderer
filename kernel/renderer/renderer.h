#pragma once
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "qlabel"
#include "types/types.h"
#include "world/world.h"
#include <barrier>
#include <functional>
#include <memory>
#include <vector>
namespace detail {
namespace renderer {
using TriangleRasterizer = std::function<void(geometry::Triangle, ZBuffer &)>;
using SegmentRasterizer = std::function<void(geometry::Segment, ZBuffer &)>;

struct RendererParameters {
  int32_t threads_count;
};

class Renderer {
public:
  Renderer(int32_t threads_count, std::vector<QRgb> &flat_screen,
           concurrency::WorkerKeeper &&worker_keeper_);

  void Render();

  void FrameSucceed();
  ZBuffer &GetZBuffer();

private:
  void ClearZBuffer();
  void RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum_matrix,
                          const M4 &camera_matrix, const camera::Camera &camera);

  int32_t total_workers;
  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;

  concurrency::WorkerKeeper worker_keeper;
};

} // namespace renderer
} // namespace detail
