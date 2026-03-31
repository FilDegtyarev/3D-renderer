#pragma once
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "light/light.h"
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
  using DirectionalLightSource = light::DirectionalLightSource;

public:
  Renderer(
      int32_t threads_total, int32_t screen_height, int32_t screen_width, Camera* camera,
      World* world, DirectionalLightSource* light, Height shadow_buffer_height,
      Width shadow_buffer_width
  );

  const Frame& MakeFrame();

private:
  WorkerKeeper MakeWorkerKeeper(
      Camera* camera, World* world, int32_t shadow_buffer_height, int32_t shadow_buffer_width
  ) const;

  void InitializeWorkers(Camera* camera, World* world, DirectionalLightSource* direction_light);

  Task
  MakeShadowMapClearTask(int32_t thread_id, World* world, DirectionalLightSource* direction_light);

  Task
  MakeShadowMapClipTask(int32_t worker_id, World* world, DirectionalLightSource* direction_light);

  Task
  MakeShadowMapDrawTask(int32_t worker_id, World* world, DirectionalLightSource* direction_light);

  Task MakeShadowMapFillGlobal(int32_t worker_id);

  Task MakeClearTask(int32_t thread_id);

  Task MakeClipFiguresTask(int32_t thread_id, Camera* camera, World* world);

  Task MakeDrawFiguresTask(
      int32_t thread_id, Camera* camera, World* world, const DirectionalLightSource* direction_light
  );

  Task MakeFillGlobalZBufferTask(int32_t thread_id);

  int32_t threads_total;
  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;
  ShadowMap shadow_map;
  Frame current_frame;
  WorkerKeeper worker_keeper;
};

} // namespace renderer
} // namespace detail
