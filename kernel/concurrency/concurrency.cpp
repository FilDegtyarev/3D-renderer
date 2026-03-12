#include "concurrency.h"
#include "camera/camera.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include "world/world.h"
#include <algorithm>
#include <cfloat>
#include <string>
#include <thread>

namespace detail {
namespace concurrency {
int32_t GetMaxThreads() { return int32_t(std::thread::hardware_concurrency()); }

inline void WorkerStorage::Clear() {
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
  //   std::fill(local_zbuffer.begin(), local_zbuffer.end(),
  //             std::vector<ZColor>(local_zbuffer[0].size(), ZColor{.color = {0, 0, 0}, .z =
  //             FLT_MAX}));
}

inline void WorkerStorage::ClearScanline() { scnaline_container.clear(); }

Worker::Worker(int32_t id_, int32_t total_workers_, int32_t sw, int32_t sh,
               WorkerStorage &self_storage_, const std::vector<WorkerStorage> &workers_storgage_,
               const camera::Camera &camera_, const world::World &world_, ZBuffer &zbuffer_,
               std::vector<QRgb> &flat_screen, std::barrier<> &barrier_)
    : id(id_), total_workers(total_workers_), screen_width(sw), screen_height(sh),
      self_storage(self_storage_), workers_storgage(workers_storgage_), camera(camera_),
      world(world_), zbuffer(zbuffer_), flat_screen(flat_screen), barrier(barrier_) {};

inline void Worker::WaitForOther() {
  // std::cout << "wait" << std::endl;
  barrier.arrive_and_wait();
  // std::cout << "no wait " << std::endl;
}

void Worker::Clear() { self_storage.Clear(); }

void Worker::ClipFigures() {
  // стадия клиппинга
  M4 camera_matrix = camera.GetCameraMatrix();
  for (const world::GlobalObject &object : world.GetObjects()) {
    int32_t block = object.TrianglesCount() / total_workers + 1;
    int32_t begin = id * block;
    int32_t end = std::min(int32_t(object.TrianglesCount()), begin + block);

    std::vector<geometry::Triangle> triangles = object.GetTrianglesForWorker(begin, end);

    for (const geometry::Triangle &triangle : triangles) {
      geometry::TriangleIntersected clipped_triangles =
          camera.ClipTriangle(triangle * camera_matrix);
      for (int32_t i = 0; i < clipped_triangles.size; ++i) {
        self_storage.clipped_triangles.push_back(clipped_triangles[i]);
      }
    }
  }
}

namespace {

inline void ViewTransform(geometry::Point &point, int32_t sw, int32_t sh) {
  point.coordinates.x = (point.X() + 1) / 2.0 * sw;
  point.coordinates.y = (point.Y() + 1) / 2.0 * sh;
}

inline void ViewSegmentTransform(geometry::Segment &segment, int32_t sw, int32_t sh) {
  ViewTransform(segment.a, sw, sh);
  ViewTransform(segment.b, sw, sh);
}

inline void ViewTriangleTransform(geometry::Triangle &triangle, int32_t sw, int32_t sh) {
  ViewTransform(triangle.a, sw, sh);
  ViewTransform(triangle.b, sw, sh);
  ViewTransform(triangle.c, sw, sh);
}
} // namespace

void Worker::DrawFigures() {
  M4 frustum_matrix = camera.GetFrustumMatrix();
  for (int32_t worker_id = 0; worker_id < total_workers; ++worker_id) {
    int32_t block = workers_storgage[worker_id].clipped_triangles.size() / total_workers + 1;
    int32_t begin = id * block;
    int32_t end =
        std::min(int32_t(workers_storgage[worker_id].clipped_triangles.size()), begin + block);

    for (int32_t triangle_index = begin; triangle_index < end; triangle_index++) {
      const geometry::Triangle clipped_triangle =
          workers_storgage[worker_id].clipped_triangles[triangle_index];
      geometry::Point a_proj = clipped_triangle.a * frustum_matrix;
      a_proj.Normalize();

      geometry::Point b_proj = clipped_triangle.b * frustum_matrix;
      b_proj.Normalize();

      geometry::Point c_proj = clipped_triangle.c * frustum_matrix;
      c_proj.Normalize();

      geometry::Triangle projective_triangle =
          geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};
      ViewTriangleTransform(projective_triangle, screen_width, screen_height);

      rasterization::DrawTriangle(projective_triangle, self_storage.local_zbuffer,
                                  self_storage.scnaline_container);
      // triangle_rasterizer(projective_triangle, local_zbuffer);
    }
  }
}

void Worker::SynchronizeZBuffers() {
  // Только синхронизируем zbuffer
  int32_t block = self_storage.local_zbuffer.GetHeight() / total_workers + 1;
  int32_t begin = id * block;
  int32_t end = std::min(int32_t(self_storage.local_zbuffer.GetHeight()), begin + block);

  for (int32_t worker_id = 0; worker_id < total_workers; ++worker_id) {
    for (int32_t row_index = begin; row_index < end; row_index++) {
      for (int32_t element_index = 0; element_index < self_storage.local_zbuffer.GetWidth();
           ++element_index) {
        if (zbuffer.At(row_index, element_index).z >
            workers_storgage[worker_id].local_zbuffer.At(row_index, element_index).z) {
          zbuffer.At(row_index, element_index) =
              workers_storgage[worker_id].local_zbuffer.At(row_index, element_index);
        }
      }
    }
  }

  for (int32_t row_index = begin; row_index < end; row_index++) {
    for (int32_t element_index = 0; element_index < screen_width; ++element_index) {
      Color c = zbuffer.At(row_index, element_index).color;
      flat_screen[row_index * screen_width + element_index] = qRgb(c.red, c.green, c.blue);
      zbuffer.At(row_index, element_index) = {{0, 0, 0}, FLT_MAX};
    }
  }
}

void Worker::FillScreenMatrix() {}

WorkerKeeper::WorkerKeeper(int32_t threads_, int32_t triangles_capacity, int32_t segments_capacity,
                           int32_t scanline_capacity, int32_t screen_width, int32_t screen_height,
                           const camera::Camera &camera_, const world::World &world_)
    : camera(camera_), world(world_), screen_height(screen_height), screen_width(screen_width),
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
    // resize(screen_height, std::vector<ZColor>(screen_width));
    workers.push_back(std::move(storage));
  }

  threads.reserve(threads_count);
}

void WorkerKeeper::UnleashWorkers(std::vector<QRgb> &flat_screen, ZBuffer &zbuffer) {
  for (int32_t worker_id = 0; worker_id < threads_count; ++worker_id) {
    Worker worker =
        Worker(worker_id, threads_count, screen_width, screen_height, workers[worker_id], workers,
               camera, world, zbuffer, flat_screen, *barrier.get());
    SpawnWorker(std::move(worker));
  }
}

void WorkerKeeper::WaitForClear() { barrier->arrive_and_wait(); }
void WorkerKeeper::WaitForClip() { barrier->arrive_and_wait(); }
void WorkerKeeper::WaitForDraw() { barrier->arrive_and_wait(); }
void WorkerKeeper::WaitForSynchronize() { barrier->arrive_and_wait(); }

void WorkerKeeper::SpawnWorker(Worker &&worker) {
  threads.emplace_back(Execute, std::move(worker));
  // std::thread thread_worker(Execute, std::move(worker));
}

void Execute(Worker worker) {
  // std::cout << "execute: " << std::endl;
  while (true) {
    // std::cout << "[worker]: wait for clear" << std::endl;
    //  std::cout << "1:" << std::endl;
    worker.Clear();
    // std::cout << "[worker]: wait for clip" << std::endl;
    worker.WaitForOther();

    worker.ClipFigures();

    // std::cout << "[worker]: wait for draw" << std::endl;
    worker.WaitForOther();
    worker.DrawFigures();

    // std::cout << "[worker]: wait for sync" << std::endl;
    worker.WaitForOther();

    worker.SynchronizeZBuffers();

    // std::cout << "[worker]: wait while master drawing" << std::endl;
    worker.WaitForOther();
  }
}

} // namespace concurrency
} // namespace detail
