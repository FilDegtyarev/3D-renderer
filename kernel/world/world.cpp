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

// GlobalObject &operator+(const V3 &vector) const;
// GlobalObject &operator*(const M3 &matrix) const;

// GlobalObject &operator+=(const V3 &vector);
// GlobalObject &operator*=(const M3 &matrix);

namespace {
inline void TriangleShift(geometry::Triangle &triangle, const V3 &vector) {
  triangle.a += vector;
  triangle.b += vector;
  triangle.c += vector;
}

inline void TriangleTransform(geometry::Triangle &triangle, const M3 &matrix) {
  triangle.a *= matrix;
  triangle.b *= matrix;
  triangle.c *= matrix;
}

inline void SegmentShift(geometry::Segment &segment, const V3 &vector) {
  segment.a += vector;
  segment.b += vector;
}

inline void SegmentTransform(geometry::Segment &segment, const M3 &matrix) {
  segment.a *= matrix;
  segment.b *= matrix;
}
} // namespace

const std::vector<GlobalObject> &World::GetObjects() const { return objects; }

} // namespace world
} // namespace detail