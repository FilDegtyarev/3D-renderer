#pragma once
#include "geometry/geometry.h"
#include "light/light.h"
#include "object.h"
#include "textures/textures.h"
#include "types/types.h"

#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace detail {
namespace world {

class GlobalObject {
  using Triangle = geometry::Triangle;
  using Segment = geometry::Segment;
  using Texture = textures::Texture;

public:
  GlobalObject(LocalObject&& local_object_, const V3& shift, const M3& transform);

  std::vector<Triangle> GetTriangles() const;
  std::vector<Triangle> GetTrianglesForWorker(int32_t begin, int32_t end) const;

  inline Triangle operator[](int32_t index) const {
    return local_object.GetTriangle(local_object.GetTriangles()[index]) * transform;
  }

  std::vector<Segment> GetSegments() const;

  int32_t TrianglesCount() const;

  const Texture& GetTexture() const;

  inline bool IsBackFaceCullingEnabled() const { return local_object.IsBackFaceCullingEnabled(); }

  Material GetMaterial() const;

private:
  LocalObject local_object;
  M4 transform;
};

class WorldBuilder;

class World {
  friend WorldBuilder;

public:
  const std::vector<GlobalObject>& GetObjects() const;

  int32_t GetTrianglesCapacity() const;
  int32_t GetSegmentCapacity() const;

  SceneBoundingBox GetBoundingBox() const;

  // void GenerateBoundingBox(const light::DirectionalLightSource& light);

private:
  World() = default;
  SceneBoundingBox MakeBoundingBox() const;
  std::vector<GlobalObject> objects;
  M4 current_transformation;
  SceneBoundingBox bounding_box;
};

class WorldBuilder {
public:
  WorldBuilder() = default;

  void AddObject(GlobalObject&& object);
  World Extract();

private:
  World world;
};

} // namespace world
} // namespace detail
