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

class Renderer {
public:
  Renderer(
      int32_t threads_count, std::vector<QRgb>& flat_screen,
      concurrency::WorkerKeeper&& worker_keeper_
  );

  void Render();

  void FrameSucceed();
  ZBuffer& GetZBuffer();

  const ZBuffer& MakeFrame();
  void ClearZBuffer();

private:
  void RenderGlobalObject(
      const world::GlobalObject& object, const M4& frustum_matrix, const M4& camera_matrix,
      const camera::Camera& camera
  );

  int32_t total_workers;
  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;

  concurrency::WorkerKeeper worker_keeper;
};

} // namespace renderer
} // namespace detail
