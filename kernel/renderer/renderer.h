#pragma once
#include "camera/camera.h"
#include "geometry/geometry.h"
#include "world/world.h"
#include <functional>
#include <vector>

namespace detail {
namespace renderer {
using TriangleRasterizer = std::function<void(geometry::Triangle, ZBuffer &)>;
using SegmentRasterizer = std::function<void(geometry::Segment, ZBuffer &)>;

class Renderer {
public:
  Renderer(ScreenHeight screen_height, ScreenWidth screen_width);

  std::vector<std::vector<Color>> Render(const world::World &world,
                                         const camera::Camera &camera);

private:
  void RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum);
  void RenderTriangle(const geometry::Triangle &triangle, const M4 &frustum);
  void RenderSegment(const geometry::Segment &segment, const M4 &frustum);

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

  size_t screen_height;
  size_t screen_width;
  ZBuffer zbuffer;
  // ScreenTriangle -> rasterize to zbuffer
  TriangleRasterizer triangle_rasterizer;
  SegmentRasterizer segment_rasterizer;
};

} // namespace renderer
} // namespace detail