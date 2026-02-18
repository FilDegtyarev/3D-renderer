#pragma once
#include "geometry/geometry.h"
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
  GlobalObject(std::unique_ptr<LocalObject> &&local_object_,
               const glm::vec3 &shift, const glm::mat3x3 &transform);

  std::vector<geometry::Triangle> GetTriangles() const;
  std::vector<geometry::Segment> GetSegments() const;

  // GlobalObject &operator*=(const M4 &matrix);

private:
  std::unique_ptr<LocalObject> local_object;
  M4 transform;
};

class World;

class WorldBuilder {
public:
  WorldBuilder();

  void AddObject(GlobalObject &&object);
  std::unique_ptr<World> Extract();

private:
  std::unique_ptr<World> world;
};

class World {
  friend WorldBuilder;

public:
  const std::vector<GlobalObject> &GetObjects() const;

  // void AddTransformation(const M4 &matrix);

private:
  World() = default;
  std::vector<GlobalObject> objects;
  M4 current_transformation;
};

} // namespace world
} // namespace detail
