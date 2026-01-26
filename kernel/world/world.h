#include "object.h"
#include "types/types.h"
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace detail {
namespace world {

class GlobalObject {
public:
  GlobalObject(const LocalObject &local_object, const glm::vec3 &shift,
               const glm::mat3x3 &transform);

private:
  LocalObject local_object;
  glm::vec3 shift;
  glm::mat3x3 transform;
};

class World {
public:
  World(const std::vector<GlobalObject> &objects);
  World(std::vector<GlobalObject> &&objects);

private:
  std::vector<GlobalObject> objects;
};

} // namespace world
} // namespace detail