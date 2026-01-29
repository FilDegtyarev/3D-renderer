#include "camera/camera.h"
#include "geometry/geometry.h"
#include "world/world.h"
#include <functional>
#include <vector>

namespace detail {
namespace renderer {
using Rasterizer = std::function<void(geometry::Triangle, ZBuffer &)>;

class Renderer {
public:
  Renderer();

  std::vector<std::vector<Color>> Render(const world::World &world,
                                         const camera::Camera &camera);

private:
  void RenderGlobalObject(const world::GlobalObject &object, const M4 &frustum);
  void RenderTriangle(const geometry::Triangle &triangle, const M4 &frustum);
  ZBuffer zbuffer;
  // Triangle -> rasterize to zbuffer
  Rasterizer rasterizer;
};

} // namespace renderer
} // namespace detail