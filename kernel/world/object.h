#pragma once
#include "geometry/geometry.h"
#include "textures/textures.h"
#include "world/object.h"
#include <vector>

namespace detail {
namespace world {

struct TriangleKeeper {
  int32_t first_index;
  int32_t second_index;
  int32_t third_index;
};

struct SegmentKeeper {
  int32_t first_index;
  int32_t second_index;
};

class LocalObjectBuilder;

class LocalObject {
  friend class LocalObjectBuilder;

public:
  LocalObject(LocalObject &&) = default;

  using Vertexes = std::vector<geometry::Point>;
  using Triangles = std::vector<TriangleKeeper>;
  using Segments = std::vector<SegmentKeeper>;

  void Normalize(float scale = 1.0);

  geometry::Triangle GetTriangle(const TriangleKeeper &trianlge) const;
  geometry::Segment GetSegment(const SegmentKeeper &segment) const;

  const Vertexes &GetVertexes() const;
  const Triangles &GetTriangles() const;
  const Segments &GetSegments() const;

private:
  LocalObject() = default;
  LocalObject(const LocalObject &local) = default;
  std::vector<geometry::Point> vertexes;
  std::vector<TriangleKeeper> triangles;
  std::vector<SegmentKeeper> segments;

  textures::Texture texture;
};

class LocalObjectBuilder {
public:
  LocalObjectBuilder();
  void AddVertex(const geometry::Point &point);
  void AddTriangle(const TriangleKeeper &triangle);
  void AddSegment(const SegmentKeeper &segment);
  void AddTexture(const textures::Texture &texture);

  LocalObject Extract();

private:
  LocalObject local_object;
};

} // namespace world

} // namespace detail
