#include "renderer.h"
#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "rasterization/rasterization.h"
#include "types/types.h"
#include <cstddef>
namespace detail {

namespace renderer {
namespace {
double EPS = 0;
void DefaultRasterizer(const geometry::Triangle &triangle, ZBuffer &zbuffer) {

  geometry::ScreenTriangle screen_triangle =
      geometry::DiscretizeTriangle(triangle);

  for (size_t height = screen_triangle.MinimumHeight();
       height <= screen_triangle.MaximumHeight(); ++height) {
    std::vector<geometry::ScreenPoint> scanline =
        rasterization::Scanline(screen_triangle, height);

    for (const auto &pixel : scanline) {
      if (pixel.z < zbuffer[pixel.x][pixel.y].z) {
        zbuffer[pixel.x][pixel.y].z = pixel.z;
        zbuffer[pixel.x][pixel.y].color = pixel.color;
      }
    }
  }
}

void EdgeRasterizer(const geometry::Segment &segment, ZBuffer &zbuffer) {
  geometry::ScreenSegment screen_segment = geometry::DiscretizeSegment(segment);
  rasterization::DrawLine(screen_segment, zbuffer);
}

} // namespace
// Rasterizer MakeDefaultRasterizer() {
//   return std::function<void(const geometry::Triangle &, ZBuffer &)>(
//       DefaultRasterizer);
// }

// Rasterizer MakeEdgeRasterizer() {
//   return std::function<void(const geometry::Triangle &, ZBuffer &)>(
//       EdgeRasterizer);
// }

Renderer::Renderer(ScreenHeight _screen_height, ScreenWidth _screen_width) {
  screen_height = _screen_height();
  screen_width = _screen_width();

  zbuffer = ZBuffer(screen_height, std::vector<ZColor>(screen_width));

  segment_rasterizer = EdgeRasterizer;
  triangle_rasterizer = nullptr;
}

std::vector<std::vector<Color>> Renderer::Render(const world::World &world,
                                                 const camera::Camera &camera) {
  M4 frsutum = camera.GetFrustumMatrix();

  for (const world::GlobalObject &object : world.GetObjects()) {
    // std::cout << "rendering..." << std::endl;
    RenderGlobalObject(object, frsutum);
    // std::cout << "done" << std::endl;
  }

  std::vector<std::vector<Color>> screen(screen_height,
                                         std::vector<Color>(screen_width));
  for (size_t i = 0; i < screen_height; ++i) {
    for (size_t j = 0; j < screen_width; ++j) {
      screen[i][j] = zbuffer[i][j].color;
    }
  }
  return screen;
}

void Renderer::RenderGlobalObject(const world::GlobalObject &object,
                                  const M4 &frustum) {
  for (const geometry::Triangle &triangle : object.GetTriangles()) {
    RenderTriangle(triangle, frustum);
  }

  for (const geometry::Segment &segment : object.GetSegments()) {
    RenderSegment(segment, frustum);
  }
}

namespace {
inline geometry::Point FromV4(const V4 &vector, Color color) {
  return geometry::Point{.x = vector.x / vector.w,
                         .y = vector.y / vector.w,
                         .z = vector.z / vector.w,
                         .color = color};
}

// void Print(geometry::Point point) {
//   std::cout << "x: " << point.x << " " << " y: " << point.y << " z: " <<
//   point.z
//             << std::endl;
// }
} // namespace

void Renderer::RenderSegment(const geometry::Segment &segment,
                             const M4 &frustum) {
  geometry::Point a_proj = FromV4(
      frustum * geometry::SwitchToProjective(segment.a), segment.a.color);

  geometry::Point b_proj = FromV4(
      frustum * geometry::SwitchToProjective(segment.b), segment.b.color);

  geometry::Segment projective_segment = geometry::Segment{a_proj, b_proj};

  ViewSegmentTransform(projective_segment);

  // rasterization::DrawLine(projective_segment.a, projective_segment.b,
  // zbuffer);
  EdgeRasterizer(projective_segment, zbuffer);
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle,
                              const M4 &frustum) {

  geometry::Point a_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.a), triangle.a.color);

  geometry::Point b_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.b), triangle.b.color);

  geometry::Point c_proj = FromV4(
      frustum * geometry::SwitchToProjective(triangle.c), triangle.c.color);

  geometry::Triangle projective_triangle =
      geometry::Triangle{.a = a_proj, .b = b_proj, .c = c_proj};

  ViewTriangleTransform(projective_triangle);

  // EdgeRasterizer(projective_triangle, zbuffer);
}

} // namespace renderer
} // namespace detail