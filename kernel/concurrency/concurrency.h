#pragma once
#include "camera/camera.h"
#include "geometry/geometry.h"
#include "qrgb.h"
#include "types/types.h"
#include "world/world.h"

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
  void Clear();
  void ClearScanline();

  std::vector<geometry::Triangle> rendering_triangles;
  std::vector<geometry::Segment> rendering_segments;

  std::vector<geometry::Triangle> clipped_triangles;
  std::vector<geometry::Segment> clipped_segments;

  std::vector<geometry::ScreenPoint> scnaline_container;
  ZBuffer local_zbuffer;
};

class Worker {
public:
  // Worker(
  //     int32_t id, int32_t total_workers_, int32_t sw, int32_t sh, WorkerStorage& self_storage_,
  //     const std::vector<WorkerStorage>& workers_storgage_, const camera::Camera& camera_,
  //     const world::World& world_, ZBuffer& zbuffer_, Frame& flat_screen, std::barrier<>& barrier_
  // );

  Worker(
      int32_t id, std::barrier<>& barrier, std::function<void(void)> clear,
      std::function<void(void)> clip_figures, std::function<void(void)> draw_figures,
      std::function<void(void)> synchronize_zbuffers, std::function<void(void)> fill_screen_matrix
  );

  void WaitForOther();

  void Clear();

  void ClipFigures();

  void DrawFigures();

  void SynchronizeZBuffers();

  void FillScreenMatrix();

  const std::vector<geometry::Triangle>& GetRenderingTriangles() const;
  const std::vector<geometry::Segment>& GetRenderingSegments() const;

private:
  int32_t id;
  std::function<void(void)> clear;
  std::function<void(void)> clip_figures;
  std::function<void(void)> draw_figures;
  std::function<void(void)> synchronize_zbuffers;
  std::function<void(void)> fill_screen_matrix;

  std::barrier<>& barrier;
};

class WorkerKeeper {
public:
  WorkerKeeper(
      int32_t threads, int32_t triangles_capacity, int32_t segments_capacity,
      int32_t scanline_capacity, int32_t screen_width, int32_t screen_height
  );

  void SpawnWorker(Worker&& worker);
  void ServeForever(Frame& flat_screen, ZBuffer& zbuffer);

  void ExecuteThreads();

  inline WorkerStorage& GetStorage(int32_t worker_id) { return workers[worker_id]; }
  std::barrier<>& GetBarrier() { return *barrier.get(); }

  static void Serve(Worker worker) {
    while (true) {
      worker.Clear();
      worker.WaitForOther();

      worker.ClipFigures();

      worker.WaitForOther();
      worker.DrawFigures();

      worker.WaitForOther();

      worker.SynchronizeZBuffers();

      worker.WaitForOther();
    }
  }

private:
  void WaitForClear();
  void WaitForClip();
  void WaitForDraw();
  void WaitForSynchronize();

  int32_t threads_count;
  int32_t screen_width;
  int32_t screen_height;

  std::vector<WorkerStorage> workers;

  std::unique_ptr<std::barrier<>> barrier;

  std::vector<std::thread> threads;
};

} // namespace concurrency
} // namespace detail
