#pragma once
#include "geometry/geometry.h"
#include "types/types.h"

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
  using Segment = geometry::Segment;
  using Triangle = geometry::Triangle;
  using ScreenPoint = geometry::ScreenPoint;

  void Clear();
  void ClearScanline();

  std::vector<Triangle> rendering_triangles;
  std::vector<Segment> rendering_segments;

  std::vector<Triangle> clipped_triangles;
  std::vector<Segment> clipped_segments;

  std::vector<ScreenPoint> scnaline_container;
  ZBuffer local_zbuffer;
};

class Worker {
  using Task = std::function<void(void)>;
  using Triangle = geometry::Triangle;
  using Segment = geometry::Segment;

public:
  Worker(
      int32_t id, std::barrier<>& barrier, Task clear, Task clip_figures, Task draw_figures,
      Task synchronize_zbuffers, Task fill_screen_matrix
  );

  void WaitForOther();

  void Clear();

  void ClipFigures();

  void DrawFigures();

  void SynchronizeZBuffers();

  void FillScreenMatrix();

private:
  int32_t id;
  Task clear;
  Task clip_figures;
  Task draw_figures;
  Task synchronize_zbuffers;
  Task fill_screen_matrix;

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
