#pragma once
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "qlabel"
#include "types/types.h"
#include "world/world.h"
#include <barrier>
#include <functional>
#include <memory>
#include <vector>
namespace detail {
namespace renderer {
using TriangleRasterizer = std::function<void(geometry::Triangle, ZBuffer &)>;
using SegmentRasterizer = std::function<void(geometry::Segment, ZBuffer &)>;

struct RendererParameters {
  int32_t threads_count;
};

class Renderer {
public:
  Renderer(int32_t threads_count, std::vector<QRgb> &flat_screen,
           concurrency::WorkerKeeper &&worker_keeper_);

  void Render();

  void FrameSucceed();
  ZBuffer &GetZBuffer();

private:
  void ClearZBuffer();
  void RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum_matrix,
                          const M4 &camera_matrix, const camera::Camera &camera);
  void RenderTriangle(const geometry::Triangle &triangle, const M4 &frustum_matrix,
                      const M4 &camera_matrix, const camera::Camera &camera);
  void RenderSegment(const geometry::Segment &segment, const M4 &frustum_matrix,
                     const M4 &camera_matrix, const camera::Camera &camera);

  inline void ViewTransform(geometry::Point &point) {
    point.x = (point.x + 1) / 2.0 * screen_width;
    point.y = (point.y + 1) / 2.0 * screen_height;
  }

  inline void ViewSegmentTransform(geometry::Segment &segment) {
    ViewTransform(segment.a);
    ViewTransform(segment.b);
  }

  inline void ViewTriangleTransform(geometry::Triangle &triangle) {
    ViewTransform(triangle.a);
    ViewTransform(triangle.b);
    ViewTransform(triangle.c);
  }

  int32_t total_workers;
  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;
  // ScreenTriangle -> rasterize to zbuffer
  TriangleRasterizer triangle_rasterizer;
  SegmentRasterizer segment_rasterizer;
  concurrency::WorkerKeeper worker_keeper;
};

} // namespace renderer
} // namespace detail
