#include "renderer.h"
#include "rasterization/algorithm.h"
#include "types/types.h"
namespace detail {

namespace renderer {
namespace {
double EPS = 0;
void DefaultRasterizer(const geometry::Triangle &triangle, ZBuffer &zbuffer) {
  for (size_t height = triangle.MinimumHeight();
       height <= triangle.MaximumHeight(); ++height) {
    std::vector<geometry::Point> scanline =
        rasterization::Scanline(triangle, height);

    for (const auto &pixel : scanline) {
      if (pixel.z < zbuffer[pixel.x][pixel.y].z) {
        zbuffer[pixel.x][pixel.y].z = pixel.z;
        zbuffer[pixel.x][pixel.y].color = pixel.color;
      }
    }
  }
}

} // namespace
Rasterizer MakeRasterizer() {
  return std::function<void(const geometry::Triangle, ZBuffer &)>(
      DefaultRasterizer);
}

std::vector<std::vector<Color>> Renderer::Render(const world::World &world,
                                                 const camera::Camera &camera) {
  M4 frsutum = camera.GetFrustumMatrix();
}

void Renderer::RenderGlobalObject(const world::GlobalObject &object,
                                  const M4 &frustum) {
  for (const geometry::Triangle &triangle : object.GetTriangles()) {
    RenderTriangle(triangle, frustum);
  }
}

void Renderer::RenderTriangle(const geometry::Triangle &triangle,
                              const M4 &frustum) {
  // transfer
}

} // namespace renderer
} // namespace detail