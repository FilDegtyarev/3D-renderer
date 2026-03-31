#include "renderer/renderer.h"

#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "shadows/shadow_map.h"
#include "types/types.h"
#include "world/world.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace detail {

namespace renderer {

Renderer::Renderer(
    int32_t threads_total, int32_t screen_height, int32_t screen_width, Camera* camera,
    World* world, DirectionalLightSource* light, Height shadow_buffer_height,
    Width shadow_buffer_width
)
    : threads_total(threads_total),
      screen_height(screen_height),
      screen_width(screen_width),
      zbuffer(Height{screen_height}, Width{screen_width}),
      shadow_map(shadow_buffer_height, shadow_buffer_width),
      current_frame(Frame(screen_height * screen_width)),
      worker_keeper(MakeWorkerKeeper(camera, world, shadow_buffer_height(), shadow_buffer_width())
      ) {
  InitializeWorkers(camera, world, light);
}

const Frame& Renderer::MakeFrame() {
  worker_keeper.Execute();
  return current_frame;
}

Renderer::WorkerKeeper Renderer::MakeWorkerKeeper(
    Camera* camera, World* world, int32_t shadow_buffer_height, int32_t shadow_buffer_width
) const {
  return WorkerKeeper(
      threads_total, world->GetTrianglesCapacity(), world->GetSegmentCapacity(), screen_width,
      screen_width, screen_height, shadow_buffer_height, shadow_buffer_width
  );
}

void Renderer::InitializeWorkers(
    Camera* camera, World* world, DirectionalLightSource* direction_light
) {
  for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
    Worker worker = Worker(
        worker_id, worker_keeper.GetBarrier(),
        MakeShadowMapClearTask(worker_id, world, direction_light),
        MakeShadowMapClipTask(worker_id, world, direction_light),
        MakeShadowMapDrawTask(worker_id, world, direction_light),
        MakeShadowMapFillGlobal(worker_id), MakeClearTask(worker_id),
        MakeClipFiguresTask(worker_id, camera, world),
        MakeDrawFiguresTask(worker_id, camera, world, direction_light),
        MakeFillGlobalZBufferTask(worker_id), nullptr
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

M4 MakeViewTransformMatrix(int32_t sw, int32_t sh) {
  M4 view_transform_matrix = 1;
  view_transform_matrix[0][0] = float(sw) / 2.0f;
  view_transform_matrix[0][3] = float(sw) / 2.0f;

  view_transform_matrix[1][1] = float(sh) / 2.0f;
  view_transform_matrix[1][3] = float(sh) / 2.0f;

  return glm::transpose(view_transform_matrix);
}

} // namespace

Task Renderer::MakeShadowMapClearTask(
    int32_t worker_id, World* world, DirectionalLightSource* direction_light
) {
  Task shadow_map_clear_task = [thread_id = worker_id, world = world, threads_count = threads_total,
                                worker_keeper = &worker_keeper, direction_light = direction_light,
                                sm_status = &sm_status]() {
    if (*sm_status == ShadowMapUpdateRequired::Not_Required) {
      return;
    }
    worker_keeper->GetStorage(thread_id).ClearShadowMap();
    worker_keeper->GetStorage(thread_id).Clear();
  };

  return shadow_map_clear_task;
}

Task Renderer::MakeShadowMapClipTask(
    int32_t worker_id, World* world, DirectionalLightSource* direction_light
) {
  Task shadow_map_clip_task = [thread_id = worker_id, world = world, threads_count = threads_total,
                               worker_keeper = &worker_keeper, direction_light = direction_light,
                               sm_status = &sm_status]() {
    if (*sm_status == ShadowMapUpdateRequired::Not_Required) {
      return;
    }

    M4 world_to_light = shadow_mapping::MakeWorldToLightMatrix(direction_light->GetBaseDirection());

    std::vector<geometry::Triangle>& self_clipped_triangles =
        worker_keeper->GetStorage(thread_id).clipped_triangles;

    geometry::TriangleIntersected first;
    geometry::TriangleIntersected second;

    uint8_t model_index = 0;
    for (const world::GlobalObject& object : world->GetObjects()) {
      int32_t block = object.TrianglesCount() / threads_count + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(object.TrianglesCount()), begin + block);

      for (int32_t index = begin; index < end; ++index) {
        geometry::Triangle triangle = object[index];

        geometry::TriangleIntersected* clipped_triangles =
            shadow_mapping::ClipTriangle(triangle * world_to_light, &first, &second);
        for (int32_t i = 0; i < clipped_triangles->size; ++i) {
          self_clipped_triangles.push_back((*clipped_triangles)[i]);
        }
      }
      ++model_index;
    }
  };

  return shadow_map_clip_task;
};

Task Renderer::MakeShadowMapDrawTask(
    int32_t worker_id, World* world, DirectionalLightSource* direction_light
) {
  Task shadow_map_draw_task = [thread_id = worker_id, threads_total = threads_total,
                               worker_keeper = &worker_keeper, world = world,
                               direction_light = direction_light, sm_status = &sm_status]() {
    if (*sm_status == ShadowMapUpdateRequired::Not_Required) {
      return;
    }

    int32_t height = worker_keeper->GetStorage(thread_id).local_shadow_map.GetHeight();

    int32_t width = worker_keeper->GetStorage(thread_id).local_shadow_map.GetWidth();

    M4 shadow_view_transform = MakeViewTransformMatrix(width, height);

    for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
      concurrency::WorkerStorage& worker_storage = worker_keeper->GetStorage(worker_id);
      concurrency::WorkerStorage& self_storage = worker_keeper->GetStorage(thread_id);

      int32_t block = worker_storage.clipped_triangles.size() / threads_total + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(worker_storage.clipped_triangles.size()), begin + block);

      for (int32_t triangle_index = begin; triangle_index < end; triangle_index++) {
        const geometry::Triangle& clipped_triangle =
            worker_storage.clipped_triangles[triangle_index];

        geometry::Triangle projective_triangle = clipped_triangle * shadow_view_transform;
        rasterization::DrawTriangleInShadowMap(
            projective_triangle, self_storage.local_shadow_map, self_storage.scnaline_container
        );
      }
    }
  };

  return shadow_map_draw_task;
}

Task Renderer::MakeShadowMapFillGlobal(int32_t worker_id) {
  Task shadow_map_fill_task = [thread_id = worker_id, threads_total = threads_total,
                               worker_keeper = &worker_keeper, shadow_map = &shadow_map,
                               current_frame = &current_frame, screen_width = screen_width,
                               sm_status = &sm_status]() {
    if (*sm_status == ShadowMapUpdateRequired::Not_Required) {
      return;
    }

    concurrency::WorkerStorage& self_storage = worker_keeper->GetStorage(thread_id);
    int32_t block = self_storage.local_shadow_map.GetHeight() / threads_total + 1;
    int32_t begin = thread_id * block;
    int32_t end = std::min(int32_t(self_storage.local_shadow_map.GetHeight()), begin + block);

    for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
      WorkerStorage& worker_storage = worker_keeper->GetStorage(worker_id);
      for (int32_t row_index = begin; row_index < end; row_index++) {
        for (int32_t element_index = 0; element_index < self_storage.local_shadow_map.GetWidth();
             ++element_index) {

          (*shadow_map)(row_index, element_index) = std::min(
              (*shadow_map)(row_index, element_index),
              worker_storage.local_shadow_map(row_index, element_index)
          );
        }
      }
    }

    self_storage.Clear();
  };

  return shadow_map_fill_task;
}

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
    M3 invt_camera_matrix = camera->GetNormalTransformMatrix();

    std::vector<geometry::Triangle>& self_clipped_triangles =
        worker_keeper->GetStorage(thread_id).clipped_triangles;

    geometry::TriangleIntersected first;
    geometry::TriangleIntersected second;

    uint8_t model_index = 0;
    for (const world::GlobalObject& object : world->GetObjects()) {
      int32_t block = object.TrianglesCount() / threads_count + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(object.TrianglesCount()), begin + block);

      bool bfc_enabled = world->GetObjects()[model_index].IsBackFaceCullingEnabled();

      for (int32_t index = begin; index < end; ++index) {
        geometry::Triangle triangle = object[index];
        triangle.model_index = model_index;

        if (bfc_enabled && geometry::IsBackFace(triangle, camera->GetCameraPosition())) {
          continue;
        }

        V3 normal_transformed(triangle.a.normal);
        normal_transformed = glm::normalize(invt_camera_matrix * normal_transformed);

        geometry::TriangleIntersected* clipped_triangles =
            camera->ClipTriangle(triangle * camera_matrix, &first, &second);
        // triangle.normal -> camera_matrix^{-t} * triangle.normal
        for (int32_t i = 0; i < clipped_triangles->size; ++i) {
          (*clipped_triangles)[i].a.normal = normal_transformed;
          (*clipped_triangles)[i].b.normal = normal_transformed;
          (*clipped_triangles)[i].c.normal = normal_transformed;
          self_clipped_triangles.push_back((*clipped_triangles)[i]);
        }
      }
      ++model_index;
    }
  };
  return clip_figures_task;
}

Task Renderer::MakeDrawFiguresTask(
    int32_t thread_id, camera::Camera* camera, world::World* world,
    const DirectionalLightSource* direction_light
) {
  Task draw_figures_task = [thread_id = thread_id, camera = camera, threads_total = threads_total,
                            worker_keeper = &worker_keeper, screen_width = screen_width,
                            screen_height = screen_height, world = world,
                            direction_light = direction_light, shadow_map = &shadow_map]() {
    int32_t shadow_height = worker_keeper->GetStorage(thread_id).local_shadow_map.GetHeight();
    int32_t shadow_width = worker_keeper->GetStorage(thread_id).local_shadow_map.GetWidth();
    M4 frustum_matrix = camera->GetFrustumMatrix();

    M4 view_transform_matrix = MakeViewTransformMatrix(screen_width, screen_height);
    M4 view_transform_inv = glm::inverse(view_transform_matrix);

    M4 frustum_to_world = glm::inverse(frustum_matrix * camera->GetCameraMatrix());

    M4 world_to_light = MakeViewTransformMatrix(shadow_width, shadow_height) *
                        shadow_mapping::MakeWorldToLightMatrix(direction_light->GetBaseDirection());
    M4 camera_inv = glm::inverse(camera->GetCameraMatrix());

    ShadowMapHelper helper;
    for (int32_t worker_id = 0; worker_id < threads_total; ++worker_id) {
      concurrency::WorkerStorage& worker_storage = worker_keeper->GetStorage(worker_id);
      concurrency::WorkerStorage& self_storage = worker_keeper->GetStorage(thread_id);

      int32_t block = worker_storage.clipped_triangles.size() / threads_total + 1;
      int32_t begin = thread_id * block;
      int32_t end = std::min(int32_t(worker_storage.clipped_triangles.size()), begin + block);

      for (int32_t triangle_index = begin; triangle_index < end; triangle_index++) {
        const geometry::Triangle& clipped_triangle =
            worker_storage.clipped_triangles[triangle_index];

        geometry::Point a_world, b_world, c_world;

        a_world = clipped_triangle.a * camera_inv;
        b_world = clipped_triangle.b * camera_inv;
        c_world = clipped_triangle.c * camera_inv;

        geometry::Triangle triangle_world = {a_world, b_world, c_world};
        float a_w, b_w, c_w;

        geometry::Point a_proj = clipped_triangle.a * frustum_matrix;
        a_w = a_proj.W();
        a_proj.Normalize();

        geometry::Point b_proj = clipped_triangle.b * frustum_matrix;
        b_w = b_proj.W();
        b_proj.Normalize();

        geometry::Point c_proj = clipped_triangle.c * frustum_matrix;
        c_w = c_proj.W();
        c_proj.Normalize();

        geometry::Triangle projective_triangle = geometry::Triangle{
            .a = a_proj,
            .b = b_proj,
            .c = c_proj,
            //.normal = clipped_triangle.normal,
            .model_index = clipped_triangle.model_index
        };

        projective_triangle.a.normal = clipped_triangle.a.normal;
        projective_triangle.b.normal = clipped_triangle.b.normal;
        projective_triangle.c.normal = clipped_triangle.c.normal;

        projective_triangle = projective_triangle * view_transform_matrix;

        helper = ShadowMapHelper{
            .shadow_map = shadow_map,
            .view_transform_inv = &view_transform_inv,
            .frustum_to_world = &frustum_to_world,
            .world_to_light = &world_to_light,
            .a_w = a_w,
            .b_w = b_w,
            .c_w = c_w,
        };
        // djn nen
        rasterization::DrawTriangle(
            projective_triangle, self_storage.local_zbuffer, self_storage.scnaline_container,
            world->GetObjects()[projective_triangle.model_index].GetTexture(), direction_light,
            &helper, triangle_world
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
        (*current_frame)[row_index * screen_width + element_index] = c;
        (*zbuffer)(row_index, element_index) = {{0}, FLT_MAX};
      }
    }
  };
  return synchronize_zbuffers_task;
}

} // namespace renderer
} // namespace detail
