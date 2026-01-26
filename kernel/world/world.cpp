#include "world.h"

namespace detail {
namespace world {
World::World(const std::vector<GlobalObject> &objects) : objects(objects) {};
World::World(std::vector<GlobalObject> &&objects)
    : objects(std::move(objects)) {};

} // namespace world
} // namespace detail