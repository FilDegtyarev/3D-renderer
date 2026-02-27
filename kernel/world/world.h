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
  GlobalObject(LocalObject &&local_object_, const glm::vec3 &shift, const glm::mat3x3 &transform);

  std::vector<geometry::Triangle> GetTriangles() const;
  std::vector<geometry::Segment> GetSegments() const;

private:
  LocalObject local_object;
  M4 transform;
};

class WorldBuilder;

class World {
  friend WorldBuilder;

public:
  const std::vector<GlobalObject> &GetObjects() const;

  int32_t GetTrianglesCapacity() const;
  int32_t GetSegmentCapacity() const;

private:
  World() = default;
  std::vector<GlobalObject> objects;
  M4 current_transformation;
};

class WorldBuilder {
public:
  WorldBuilder();

  void AddObject(GlobalObject &&object);
  World Extract();

private:
  World world;
};

} // namespace world
} // namespace detail
