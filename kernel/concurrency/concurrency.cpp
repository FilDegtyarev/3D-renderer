#include "concurrency.h"

#include "types/types.h"

#include <cfloat>
#include <thread>

namespace detail {
namespace concurrency {
int32_t GetMaxThreads() {
  return int32_t(std::thread::hardware_concurrency());
}

void WorkerStorage::Clear() {
  rendering_triangles.clear();
  clipped_triangles.clear();
  rendering_segments.clear();
  clipped_segments.clear();
  scnaline_container.clear();

  for (int i = 0; i < local_zbuffer.GetHeight(); ++i) {
    for (int j = 0; j < local_zbuffer.GetWidth(); ++j) {
      local_zbuffer(i, j).color = {0, 0, 0};
      local_zbuffer(i, j).z = FLT_MAX;
    }
  }
}

void WorkerStorage::ClearScanline() {
  scnaline_container.clear();
}

Worker::Worker(
    int32_t id, std::barrier<>& barrier, Task clear, Task clip_figures, Task draw_figures,
    Task synchronize_zbuffers, Task fill_screen_matrix
)
    : id(id),
      barrier(barrier),
      clear(clear),
      clip_figures(clip_figures),
      draw_figures(draw_figures),
      synchronize_zbuffers(synchronize_zbuffers),
      fill_screen_matrix(nullptr) {};

inline void Worker::WaitForOther() {
  barrier.arrive_and_wait();
}

void Worker::Clear() {
  clear();
}

void Worker::ClipFigures() {
  clip_figures();
}

void Worker::DrawFigures() {
  draw_figures();
}

void Worker::FillGlobalZBuffer() {
  synchronize_zbuffers();
}

void Worker::FillScreenMatrix() {}

WorkerKeeper::WorkerKeeper(
    int32_t threads_count, int32_t triangles_capacity, int32_t segments_capacity,
    int32_t scanline_capacity, int32_t screen_width, int32_t screen_height
)
    : barrier(std::make_unique<std::barrier<>>(threads_count + 1)) {
  workers = std::vector<WorkerStorage>(0);

  for (size_t worker = 0; worker < threads_count; ++worker) {
    WorkerStorage storage;
    storage.rendering_triangles.reserve(triangles_capacity);
    storage.clipped_triangles.reserve(12 * triangles_capacity);

    storage.rendering_segments.reserve(segments_capacity);
    storage.clipped_segments.reserve(segments_capacity);

    storage.scnaline_container.reserve(scanline_capacity);
    storage.local_zbuffer = ZBuffer(Height{screen_height}, Width{screen_width});
    workers.push_back(std::move(storage));
  }

  threads.reserve(threads_count);
}

void WorkerKeeper::SpawnWorker(Worker&& worker) {
  threads.emplace_back(WorkerKeeper::Serve, std::move(worker));
}

void WorkerKeeper::Execute() {
  WaitForClear();
  WaitForClip();
  WaitForDraw();
  WaitForSynchronize();
}

void WorkerKeeper::WaitForClear() {
  barrier->arrive_and_wait();
}
void WorkerKeeper::WaitForClip() {
  barrier->arrive_and_wait();
}
void WorkerKeeper::WaitForDraw() {
  barrier->arrive_and_wait();
}
void WorkerKeeper::WaitForSynchronize() {
  barrier->arrive_and_wait();
}

} // namespace concurrency
} // namespace detail
