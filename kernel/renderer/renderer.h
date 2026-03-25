#pragma once
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "qlabel"
#include "types/types.h"
#include "world/world.h"

namespace detail {
namespace renderer {

class Renderer {
  using WorkerKeeper = concurrency::WorkerKeeper;
  using Camera = camera::Camera;
  using World = world::World;
  using WorkerStorage = concurrency::WorkerStorage;
  using Worker = concurrency::Worker;

public:
  Renderer(
      int32_t threads_total, int32_t screen_height, int32_t screen_width,
      WorkerKeeper&& worker_keeper
  );

  const Frame& MakeFrame();

  void UnleashWorkers(Camera* camera, World* world);

private:
  Task MakeClearTask(int32_t thread_id);
  Task MakeClipFiguresTask(int32_t thread_id, Camera* camera, World* world);
  Task MakeDrawFiguresTask(int32_t thread_id, Camera* camera, World* world);
  Task MakeSynchronizeZBuffersTask(int32_t thread_id);

  int32_t threads_total;
  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;
  Frame current_frame;
  WorkerKeeper worker_keeper;
};

} // namespace renderer
} // namespace detail
