#pragma once
#include "camera/camera.h"
#include "geometry/geometry.h"
#include "qrgb.h"
#include "types/types.h"
#include "world/world.h"
#include <atomic>
#include <barrier>
#include <cassert>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace detail {
namespace renderer {
class Renderer;
}

namespace concurrency {
int32_t GetMaxThreads();

struct WorkerStorage {
  inline void Clear();
  inline void ClearScanline();

  std::vector<geometry::Triangle> rendering_triangles;
  std::vector<geometry::Segment> rendering_segments;

  std::vector<geometry::Triangle> clipped_triangles;
  std::vector<geometry::Segment> clipped_segments;

  std::vector<geometry::ScreenPoint> scnaline_container;
  ZBuffer local_zbuffer;
};

class Worker {
public:
  Worker(int32_t id, int32_t total_workers_, int32_t sw, int32_t sh, WorkerStorage &self_storage_,
         const std::vector<WorkerStorage> &workers_storgage_, const camera::Camera &camera_,
         const world::World &world_, ZBuffer &zbuffer_, std::vector<QRgb> &flat_screen,
         std::barrier<> &barrier_);

  inline void WaitForOther();

  void Clear();

  void ClipFigures();

  void DrawFigures();

  void SynchronizeZBuffers();

  void FillScreenMatrix();

  const std::vector<geometry::Triangle> &GetRenderingTriangles() const;
  const std::vector<geometry::Segment> &GetRenderingSegments() const;
  int32_t id;

private:
  int32_t total_workers;

  int32_t screen_width;
  int32_t screen_height;

  const std::vector<WorkerStorage> &workers_storgage;
  WorkerStorage &self_storage;
  const camera::Camera &camera;
  const world::World &world;
  ZBuffer &zbuffer;
  std::vector<QRgb> &flat_screen;
  std::barrier<> &barrier;
};

class WorkerKeeper {
  friend class renderer::Renderer;

public:
  WorkerKeeper(int32_t threads, int32_t triangles_capacity, int32_t segments_capacity,
               int32_t scanline_capacity, int32_t screen_width, int32_t screen_height,
               const camera::Camera &camera_, const world::World &world_);

  void UnleashWorkers(std::vector<QRgb> &flat_screen, ZBuffer &zbuffer);
  void WaitForClear();
  void WaitForClip();
  void WaitForDraw();
  void WaitForSynchronize();

private:
  void SpawnWorker(Worker &&worker);

  int32_t threads_count;
  int32_t screen_width;
  int32_t screen_height;

  std::vector<WorkerStorage> workers;

  const camera::Camera &camera;
  const world::World &world;

  std::unique_ptr<std::barrier<>> barrier;

  std::vector<std::thread> threads;
};

void Execute(Worker);

} // namespace concurrency
} // namespace detail
