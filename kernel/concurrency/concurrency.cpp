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
/*
std::vector<geometry::Triangle> rendering_triangles;
std::vector<geometry::Segment> rendering_segments;

std::vector<geometry::Triangle> clipped_triangles;
std::vector<geometry::Segment> clipped_segments;

std::vector<geometry::ScreenPoint> scnaline_container;
ZBuffer local_zbuffer;*/
int32_t GetMaxThreads() { return int32_t(std::thread::hardware_concurrency()); }

inline void WorkerStorage::Clear() {
  rendering_triangles.clear();
  clipped_triangles.clear();
  rendering_segments.clear();
  clipped_segments.clear();
  scnaline_container.clear();

  for (int i = 0; i < local_zbuffer.size(); ++i) {
    for (int j = 0; j < local_zbuffer[i].size(); ++j) {
      local_zbuffer[i][j].color = {0, 0, 0};
      local_zbuffer[i][j].z = DBL_MAX;
    }
  }
  //   std::fill(local_zbuffer.begin(), local_zbuffer.end(),
  //             std::vector<ZColor>(local_zbuffer[0].size(), ZColor{.color = {0, 0, 0}, .z =
  //             DBL_MAX}));
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
    for (int32_t i = id; i < object.GetTriangles().size(); i += total_workers) {
      const geometry::Triangle triangle = object.GetTriangles()[i] * camera_matrix;
      // self_storage.clipped_triangles.push_back(triangle);
      //  self_storage.clipped_triangles.push_back(object.GetTriangles()[i]);
      // continue;
      std::vector<geometry::Triangle> clipped_triangles = camera.ClipTriangle(triangle);
      self_storage.clipped_triangles.insert(self_storage.clipped_triangles.end(),
                                            clipped_triangles.begin(), clipped_triangles.end());
    }
  }
}

namespace {
inline geometry::Point FromV4(const V4 &vector, Color color) {
  return geometry::Point{
      .x = vector.x / vector.w, .y = vector.y / vector.w, .z = vector.z / vector.w, .color = color};
}
inline void ViewTransform(geometry::Point &point, int32_t sw, int32_t sh) {
  point.x = (point.x + 1) / 2.0 * sw;
  point.y = (point.y + 1) / 2.0 * sh;
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

    // std::cout << "[worker]: drawing " << workers_storgage[worker_id].clipped_triangles.size()
    //           << std::endl;
    for (int32_t triangle_index = id;
         triangle_index < workers_storgage[worker_id].clipped_triangles.size();
         triangle_index += total_workers) {
      const geometry::Triangle clipped_triangle =
          workers_storgage[worker_id].clipped_triangles[triangle_index];
      geometry::Point a_proj =
          FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.a),
                 clipped_triangle.a.color);

      geometry::Point b_proj =
          FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.b),
                 clipped_triangle.b.color);

      geometry::Point c_proj =
          FromV4(frustum_matrix * geometry::SwitchToProjective(clipped_triangle.c),
                 clipped_triangle.c.color);

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
  for (int32_t worker_id = 0; worker_id < total_workers; ++worker_id) {
    for (int32_t row_index = id; row_index < self_storage.local_zbuffer.size();
         row_index += total_workers) {
      // выбрали строку. Теперь выбираем что туда сувать
      for (int32_t element_index = 0; element_index < self_storage.local_zbuffer[0].size();
           ++element_index) {
        if (zbuffer[row_index][element_index].z >
            workers_storgage[worker_id].local_zbuffer[row_index][element_index].z) {
          zbuffer[row_index][element_index] =
              workers_storgage[worker_id].local_zbuffer[row_index][element_index];
        }
      }
    }
  }
  for (int32_t row_index = id; row_index < screen_height; row_index += total_workers) {
    for (int32_t element_index = 0; element_index < screen_width; ++element_index) {
      Color c = zbuffer[row_index][element_index].color;
      flat_screen[row_index * screen_width + element_index] = qRgb(c.red, c.green, c.blue);
      zbuffer[row_index][element_index] = {{0, 0, 0}, DBL_MAX};
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
    storage.local_zbuffer.resize(screen_height, std::vector<ZColor>(screen_width));
    workers.push_back(std::move(storage));
  }

  threads.reserve(threads_count);
}

void WorkerKeeper::UnleashWorkers(std::vector<QRgb> &flat_screen, ZBuffer &zbuffer) {
  /*
  Worker(int32_t id, int32_t total_workers_, int32_t sw, int32_t sh, WorkerStorage &self_storage_,
  const std::vector<WorkerStorage> &workers_storgage_, const camera::Camera &camera_, const
  world::World &world_, ZBuffer &zbuffer_);
  */
  // std::cout << "Unleash!!!" << std::endl;
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

auto N = std::chrono::high_resolution_clock::now;
// void Execute(Worker worker) {
//   // std::cout << "execute: " << std::endl;
//   while (true) {
//     // std::cout << "[worker]: wait for clear" << std::endl;
//     //  std::cout << "1:" << std::endl;
//     worker.Clear();
//     // std::cout << "[worker]: wait for clip" << std::endl;
//     worker.WaitForOther();

//     worker.ClipFigures();

//     // std::cout << "[worker]: wait for draw" << std::endl;
//     worker.WaitForOther();
//     worker.DrawFigures();

//     // std::cout << "[worker]: wait for sync" << std::endl;
//     worker.WaitForOther();

//     worker.SynchronizeZBuffers();

//     // std::cout << "[worker]: wait while master drawing" << std::endl;
//     worker.WaitForOther();
//   }
// }

void Execute(Worker worker) {
  using clock = std::chrono::high_resolution_clock;

  // Накопители времени в микросекундах
  uint64_t t_clear = 0, t_clip = 0, t_draw = 0, t_sync = 0;
  uint64_t t_wait_total = 0;
  uint32_t frame_count = 0;

  while (true) {
    auto t_frame_start = clock::now();

    // --- CLEAR ---
    auto s0 = clock::now();
    worker.Clear();
    t_clear += std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - s0).count();

    // --- WAIT FOR CLIP ---
    auto w0 = clock::now();
    worker.WaitForOther();
    t_wait_total +=
        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - w0).count();

    // --- CLIP ---
    auto s1 = clock::now();
    worker.ClipFigures();
    t_clip += std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - s1).count();

    // --- WAIT FOR DRAW ---
    auto w1 = clock::now();
    worker.WaitForOther();
    t_wait_total +=
        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - w1).count();

    // --- DRAW ---
    auto s2 = clock::now();
    worker.DrawFigures();
    t_draw += std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - s2).count();

    // --- WAIT FOR SYNC ---
    auto w2 = clock::now();
    worker.WaitForOther();
    t_wait_total +=
        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - w2).count();

    // --- SYNC ---
    auto s3 = clock::now();
    worker.SynchronizeZBuffers();
    t_sync += std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - s3).count();

    // --- WAIT WHILE MASTER DRAWING ---
    auto w3 = clock::now();
    worker.WaitForOther();
    t_wait_total +=
        std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - w3).count();

    frame_count++;

    // Вывод статистики раз в 100 итераций
    if (frame_count >= 10) {
      auto avg = [](uint64_t total) { return total / 100.0 / 1000.0; }; // в миллисекунды

      std::string report = "\n[Worker " + std::to_string(worker.id) + "] Avg Times (ms):\n";
      report += "  Work:  Clear: " + std::to_string(avg(t_clear)) +
                " | Clip: " + std::to_string(avg(t_clip)) +
                " | Draw: " + std::to_string(avg(t_draw)) +
                " | Sync: " + std::to_string(avg(t_sync)) + "\n";
      report += "  Wait:  Total Idle: " + std::to_string(avg(t_wait_total)) + " ms\n";
      report +=
          "  Total: " + std::to_string(avg(t_clear + t_clip + t_draw + t_sync + t_wait_total)) +
          " ms\n";
      report += "-----------------------------------";

      std::cout << report << std::endl;

      // Сброс счетчиков
      t_clear = t_clip = t_draw = t_sync = t_wait_total = 0;
      frame_count = 0;
    }
  }
}
} // namespace concurrency
} // namespace detail
