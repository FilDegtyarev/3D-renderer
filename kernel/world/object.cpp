#include "object.h"

#include "geometry/geometry.h"
#include "types/types.h"

#include <cstdio>

namespace detail {
namespace world {

void LocalObject::Normalize(float scale) {
  float max = -1.0;
  for (const geometry::Point& point : vertexes) {
    max = std::max(max, abs(point.X()));
    max = std::max(max, abs(point.Y()));
    max = std::max(max, abs(point.Z()));
  }
  max /= scale;

  for (geometry::Point& point : vertexes) {
    point.Scale(1.0f / max);
  }
}

namespace {
inline bool IsNormalDifferent(const TriangleInfo& info) {
  return info.first.normal_number != -1;
}
}; // namespace

LocalObject::Triangle LocalObject::GetTriangle(const TriangleInfo& triangle) const {
  geometry::Triangle result = {
      vertexes[triangle.first.vertex_number], vertexes[triangle.second.vertex_number],
      vertexes[triangle.third.vertex_number]
  };
  // geometry::Point a = ;
  // geometry::Point b = ;
  // geometry::Point c = ;

  if (texture.IsActive()) {
    result.a.texture_coordinates = texture.GetTextureCoordinates(triangle.first.texture_number);
    result.b.texture_coordinates = texture.GetTextureCoordinates(triangle.second.texture_number);
    result.c.texture_coordinates = texture.GetTextureCoordinates(triangle.third.texture_number);
  }

  if (triangle.first.normal_number == -1 || triangle.second.normal_number == -1 ||
      triangle.third.normal_number == -1) {
    V3 normal = glm::normalize(geometry::MakeNormal(result));
    result.a.normal = normal;
    result.b.normal = normal;
    result.c.normal = normal;
    return result;
  } else {
    result.a.normal = normals[triangle.first.normal_number];
    result.b.normal = normals[triangle.second.normal_number];
    result.c.normal = normals[triangle.third.normal_number];
    return result;
  }
}

LocalObject::Segment LocalObject::GetSegment(const SegmentKeeper& keeper) const {
  return geometry::Segment{vertexes[keeper.first_index], vertexes[keeper.second_index]};
}

const LocalObject::Vertexes& LocalObject::GetVertexes() const {
  return vertexes;
}

const LocalObject::Triangles& LocalObject::GetTriangles() const {
  return triangles;
}

const LocalObject::Segments& LocalObject::GetSegments() const {
  return segments;
}

const LocalObject::Texture& LocalObject::GetTexture() const {
  return texture;
}

void LocalObjectBuilder::AddVertex(const geometry::Point& point) {
  local_object.vertexes.push_back(point);
}

void LocalObjectBuilder::AddTriangle(const TriangleInfo& triangle) {
  local_object.triangles.push_back(triangle);
}

void LocalObjectBuilder::AddSegment(const SegmentKeeper& segment) {
  local_object.segments.push_back(segment);
}

void LocalObjectBuilder::AddTexture(const LocalObject::Texture& texture) {
  local_object.texture = texture;
}

void LocalObjectBuilder::AddNormal(const V3& normal) {
  local_object.normals.push_back(normal);
}

void LocalObjectBuilder::SetBackFaceCullingMode(BackFaceCullingStatus status) {
  local_object.back_face_culling_status = status;
}

LocalObject LocalObjectBuilder::Extract() {
  return std::move(local_object);
}

} // namespace world
} // namespace detail
