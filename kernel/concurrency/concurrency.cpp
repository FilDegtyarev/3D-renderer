#include "concurrency.h"

#include "camera/camera.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include "world/world.h"

#include <algorithm>
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
      local_zbuffer.At(i, j).color = {0, 0, 0};
      local_zbuffer.At(i, j).z = FLT_MAX;
    }
  }
}

void WorkerStorage::ClearScanline() {
  scnaline_container.clear();
}

Worker::Worker(
    int32_t id, std::barrier<>& barrier, std::function<void(void)> clear,
    std::function<void(void)> clip_figures, std::function<void(void)> draw_figures,
    std::function<void(void)> synchronize_zbuffers, std::function<void(void)> fill_screen_matrix
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

void Worker::SynchronizeZBuffers() {
  synchronize_zbuffers();
}

void FillScreenMatrix() {
  return;
}

namespace {

inline void ViewTransform(geometry::Point& point, int32_t sw, int32_t sh) {
  point.coordinates.x = (point.X() + 1) / 2.0 * sw;
  point.coordinates.y = (point.Y() + 1) / 2.0 * sh;
}

inline void ViewSegmentTransform(geometry::Segment& segment, int32_t sw, int32_t sh) {
  ViewTransform(segment.a, sw, sh);
  ViewTransform(segment.b, sw, sh);
}

inline void ViewTriangleTransform(geometry::Triangle& triangle, int32_t sw, int32_t sh) {
  ViewTransform(triangle.a, sw, sh);
  ViewTransform(triangle.b, sw, sh);
  ViewTransform(triangle.c, sw, sh);
}
} // namespace

void Worker::FillScreenMatrix() {}

WorkerKeeper::WorkerKeeper(
    int32_t threads_, int32_t triangles_capacity, int32_t segments_capacity,
    int32_t scanline_capacity, int32_t screen_width, int32_t screen_height
)
    : screen_height(screen_height),
      screen_width(screen_width),
      barrier(std::make_unique<std::barrier<>>(threads_ + 1)) {
  threads_count = threads_;
  workers = std::vector<WorkerStorage>(0);

  for (size_t worker = 0; worker < threads_count; ++worker) {
    WorkerStorage storage;
    storage.rendering_triangles.reserve(triangles_capacity);
    storage.clipped_triangles.reserve(12 * triangles_capacity);

    storage.rendering_segments.reserve(segments_capacity);
    storage.clipped_segments.reserve(segments_capacity);

    storage.scnaline_container.reserve(scanline_capacity);
    storage.local_zbuffer = ZBuffer(screen_height, screen_width);
    workers.push_back(std::move(storage));
  }

  threads.reserve(threads_count);
}

void WorkerKeeper::ServeForever(Frame& flat_screen, ZBuffer& zbuffer) {}

void WorkerKeeper::ExecuteThreads() {
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

void WorkerKeeper::SpawnWorker(Worker&& worker) {
  threads.emplace_back(WorkerKeeper::Serve, std::move(worker));
}

} // namespace concurrency
} // namespace detail
