#include "geometry/geometry.h"
#include "world/world.h"
#include <functional>
#include <vector>

namespace detail {
namespace renderer {

namespace {
using Points = std::vector<geometry::Point>;
using ZBuffer = std::vector<std::vector<geometry::Point>>;
using Rasterizer = std::function<void(geometry::Triangle, ZBuffer &)>;

void DefaultRasterizer(geometry::Triangle triangle, ZBuffer &zbuffer);

Rasterizer CreateDefault();
} // namespace

class Renderer {
public:
  Renderer();

  std::vector<std::vector<Color>> Render(const world::World &world);

private:
  ZBuffer zbuffer;
  // Triangle -> rasterize to zbuffer
  std::function<void(geometry::Triangle, ZBuffer &)> rasterizer;
};

} // namespace renderer
} // namespace detail