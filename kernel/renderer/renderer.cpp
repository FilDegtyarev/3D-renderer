#include "renderer/renderer.h"

#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include "world/world.h"

#include <cstddef>
#include <cstdint>
namespace detail {

namespace renderer {

Renderer::Renderer(
    int32_t threads_total, int32_t screen_height, int32_t screen_width, Camera* camera, World* world
)
    : threads_total(threads_total),
      screen_height(screen_height),
      screen_width(screen_width),
      zbuffer(Height{screen_height}, Width{screen_width}),
      current_frame(Frame(screen_height * screen_width)),
      worker_keeper(MakeWorkerKeeper(camera, world)) {
  InitializeWorkers(camera, world);
}

const Frame& Renderer::MakeFrame() {
  worker_keeper.Execute();
  return current_frame;
}

Renderer::WorkerKeeper Renderer::MakeWorkerKeeper(Camera* camera, World* world) const {
  return WorkerKeeper(
      threads_total, world->GetTrianglesCapacity(), world->GetSegmentCapacity(), screen_width,
      screen_width, screen_height
  );
}

void Renderer::InitializeWorkers(Camera* camera, World* world) {
  for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
    Worker worker = Worker(
        worker_id, worker_keeper.GetBarrier(), MakeClearTask(worker_id),
        MakeClipFiguresTask(worker_id, camera, world),
        MakeDrawFiguresTask(worker_id, camera, world), MakeFillGlobalZBufferTask(worker_id), nullptr
    );

    worker_keeper.SpawnWorker(std::move(worker));
  }
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

Task Renderer::MakeClearTask(int32_t worker_id) {
  Task clear_task = [thread_id = worker_id, worker_keeper = &worker_keeper]() {
    worker_keeper->GetStorage(thread_id).Clear();
  };
  return clear_task;
}

Task Renderer::MakeClipFiguresTask(int32_t thread_id, Camera* camera, World* world) {
  Task clip_figures_task = [thread_id = thread_id, camera = camera, world = world,
                            threads_count = threads_total, worker_keeper = &worker_keeper]() {
    M4 camera_matrix = camera->GetCameraMatrix();
    std::vector<geometry::Triangle>& self_clipped_triangles =
        worker_keeper->GetStorage(thread_id).clipped_triangles;

    geometry::TriangleIntersected first;
    geometry::TriangleIntersected second;

    for (const world::GlobalObject& object : world->GetObjects()) {
      int32_t block = object.TrianglesCount() / threads_count + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(object.TrianglesCount()), begin + block);

      for (int32_t index = begin; index < end; ++index) {
        auto triangle = object[index];
        geometry::TriangleIntersected* clipped_triangles =
            camera->ClipTriangle(triangle * camera_matrix, &first, &second);
        for (int32_t i = 0; i < clipped_triangles->size; ++i) {
          self_clipped_triangles.push_back((*clipped_triangles)[i]);
        }
      }
    }
  };
  return clip_figures_task;
}

Task Renderer::MakeDrawFiguresTask(int32_t thread_id, camera::Camera* camera, world::World* world) {
  Task draw_figures_task = [thread_id = thread_id, camera = camera, threads_total = threads_total,
                            worker_keeper = &worker_keeper, screen_width = screen_width,
                            screen_height = screen_height, world = world]() {
    M4 frustum_matrix = camera->GetFrustumMatrix();
    for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
      concurrency::WorkerStorage& worker_storage = worker_keeper->GetStorage(worker_id);
      concurrency::WorkerStorage& self_storage = worker_keeper->GetStorage(thread_id);

      int32_t block = worker_storage.clipped_triangles.size() / threads_total + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(worker_storage.clipped_triangles.size()), begin + block);

      for (int32_t triangle_index = begin; triangle_index < end; triangle_index++) {
        const geometry::Triangle clipped_triangle =
            worker_storage.clipped_triangles[triangle_index];
        geometry::Point a_proj = clipped_triangle.a * frustum_matrix;
        a_proj.Normalize();

        geometry::Point b_proj = clipped_triangle.b * frustum_matrix;
        b_proj.Normalize();

        geometry::Point c_proj = clipped_triangle.c * frustum_matrix;
        c_proj.Normalize();

        geometry::Triangle projective_triangle =
            geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};
        ViewTriangleTransform(projective_triangle, screen_width, screen_height);

        rasterization::DrawTriangle(
            projective_triangle, self_storage.local_zbuffer, self_storage.scnaline_container,
            world->GetObjects()[0].GetTexture()
        );
      }
    }
  };
  return draw_figures_task;
}

Task Renderer::MakeFillGlobalZBufferTask(int32_t thread_id) {
  Task synchronize_zbuffers_task = [thread_id = thread_id, threads_total = threads_total,
                                    worker_keeper = &worker_keeper, zbuffer = &zbuffer,
                                    current_frame = &current_frame, screen_width = screen_width]() {
    concurrency::WorkerStorage& self_storage = worker_keeper->GetStorage(thread_id);
    int32_t block = self_storage.local_zbuffer.GetHeight() / threads_total + 1;
    int32_t begin = thread_id * block;
    int32_t end = std::min(int32_t(self_storage.local_zbuffer.GetHeight()), begin + block);

    for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
      WorkerStorage& worker_storage = worker_keeper->GetStorage(worker_id);
      for (int32_t row_index = begin; row_index < end; row_index++) {
        for (int32_t element_index = 0; element_index < self_storage.local_zbuffer.GetWidth();
             ++element_index) {
          if ((*zbuffer)(row_index, element_index).z >
              worker_storage.local_zbuffer(row_index, element_index).z) {
            (*zbuffer)(row_index, element_index) =
                worker_storage.local_zbuffer(row_index, element_index);
          }
        }
      }
    }

    for (int32_t row_index = begin; row_index < end; row_index++) {
      for (int32_t element_index = 0; element_index < screen_width; ++element_index) {
        Color c = (*zbuffer)(row_index, element_index).color;
        (*current_frame)[row_index * screen_width + element_index] = {c.red, c.green, c.blue};
        (*zbuffer)(row_index, element_index) = {{0, 0, 0}, FLT_MAX};
      }
    }
  };
  return synchronize_zbuffers_task;
}

} // namespace renderer
} // namespace detail
