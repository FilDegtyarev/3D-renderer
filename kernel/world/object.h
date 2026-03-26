#pragma once
#include "geometry/geometry.h"
#include "textures/textures.h"
#include "types/types.h"

#include <vector>

namespace detail {
namespace world {

struct TriangleInfo {
  VertexInfo first;
  VertexInfo second;
  VertexInfo third;
};

struct SegmentKeeper {
  int32_t first_index;
  int32_t second_index;
};

class LocalObjectBuilder;

class LocalObject {
  friend class LocalObjectBuilder;

  using Triangle = geometry::Triangle;
  using Segment = geometry::Segment;
  using Point = geometry::Point;
  using Texture = textures::Texture;

public:
  LocalObject(LocalObject&&) = default;

  using Vertexes = std::vector<geometry::Point>;
  using Triangles = std::vector<TriangleInfo>;
  using Segments = std::vector<SegmentKeeper>;

  void Normalize(float scale = 1.0);

  Triangle GetTriangle(const TriangleInfo& trianlge) const;
  Segment GetSegment(const SegmentKeeper& segment) const;

  const Vertexes& GetVertexes() const;
  const Triangles& GetTriangles() const;
  const Segments& GetSegments() const;

  const textures::Texture& GetTexture() const;

private:
  LocalObject() = default;
  LocalObject(const LocalObject& local) = default;
  std::vector<Point> vertexes;
  std::vector<TriangleInfo> triangles;
  std::vector<SegmentKeeper> segments;

  Texture texture;
};

class LocalObjectBuilder {
public:
  LocalObjectBuilder() = default;

  void AddVertex(const geometry::Point& point);
  void AddTriangle(const TriangleInfo& vertex_info);
  void AddSegment(const SegmentKeeper& segment);
  void AddTexture(const textures::Texture& texture);

  LocalObject Extract();

private:
  LocalObject local_object;
};

} // namespace world

} // namespace detail
