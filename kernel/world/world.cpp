#include "world.h"

namespace detail {
namespace world {
World::World(const std::vector<GlobalObject> &objects) : objects(objects) {};
World::World(std::vector<GlobalObject> &&objects)
    : objects(std::move(objects)) {};

GlobalObject::GlobalObject(const LocalObject &local_object_,
                           const glm::vec3 &shift_,
                           const glm::mat3x3 &transform_)
    : local_object(local_object_), shift(shift_), transform(transform_) {};

const std::vector<geometry::Triangle> &GlobalObject::GetTriangles() const {
  // ИСПРАВИТЬ
  return local_object.GetTriangles();
}

const std::vector<geometry::Segment> &GlobalObject::GetSegments() const {
  return local_object.GetSegments();
}

const std::vector<GlobalObject> &World::GetObjects() const { return objects; }

} // namespace world
} // namespace detail