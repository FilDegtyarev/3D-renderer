#include "object.h"

namespace detail {
namespace world {

void LocalObject::Normalize(double scale) {
  double max = -1.0;
  for (const geometry::Point &point : vertexes) {
    max = std::max(max, abs(point.x));
    max = std::max(max, abs(point.y));
    max = std::max(max, abs(point.z));
  }
  max /= scale;

  for (geometry::Point &point : vertexes) {
    point *= (1.0 / max);
  }
}

geometry::Triangle
LocalObject::GetTriangle(const TriangleKeeper &triangle) const {
  return geometry::Triangle{vertexes[triangle.first_index],
                            vertexes[triangle.second_index],
                            vertexes[triangle.third_index]};
}

geometry::Segment LocalObject::GetSegment(const SegmentKeeper &keeper) const {
  return geometry::Segment{vertexes[keeper.first_index],
                           vertexes[keeper.second_index]};
}

const LocalObject::Vertexes &LocalObject::GetVertexes() const {
  return vertexes;
}

const LocalObject::Triangles &LocalObject::GetTriangles() const {
  return triangles;
}

const LocalObject::Segments &LocalObject::GetSegments() const {
  return segments;
}

LocalObjectBuilder::LocalObjectBuilder() {
  local_object = std::make_unique<LocalObject>(LocalObject());
};

void LocalObjectBuilder::AddVertex(const geometry::Point &point) {
  local_object->vertexes.push_back(point);
}

void LocalObjectBuilder::AddTriangle(const TriangleKeeper &triangle) {
  local_object->triangles.push_back(triangle);
}

void LocalObjectBuilder::AddSegment(const SegmentKeeper &segment) {
  local_object->segments.push_back(segment);
}

std::unique_ptr<LocalObject> &&LocalObjectBuilder::Extract() {
  return std::move(local_object);
}

} // namespace world
} // namespace detail