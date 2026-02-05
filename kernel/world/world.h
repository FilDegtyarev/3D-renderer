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

  // GlobalObject operator+(const V3 &vector) const;
  // GlobalObject operator*(const M3 &matrix) const;

  GlobalObject &operator+=(const V3 &vector);
  GlobalObject &operator*=(const M3 &matrix);

private:
  std::unique_ptr<LocalObject> local_object;
  glm::vec3 shift;
  glm::mat3x3 transform;
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

private:
  World() = default;
  std::vector<GlobalObject> objects;
};

} // namespace world
} // namespace detail