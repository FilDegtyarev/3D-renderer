#include "world.h"

namespace detail {
namespace world {
World::World(const std::vector<GlobalObject> &objects) : objects(objects) {};
World::World(std::vector<GlobalObject> &&objects)
    : objects(std::move(objects)) {};

const std::vector<geometry::Triangle> &GlobalObject::GetTriangles() const {
  // ИСПРАВИТЬ
  return local_object.GetTriangles();
}
} // namespace world
} // namespace detail