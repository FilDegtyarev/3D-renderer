#include "geometry.h"
#include <algorithm>
#include <vector>

namespace detail {
namespace geometry {

namespace {
inline bool IsHigher(const Point &left, const Point &right) {
  if (left.y > right.y) {
    return true;
  } else if (left.y == right.y && left.x >= right.x) {
    return true;
  }
  return false;
}

struct HeightComparator {
  bool operator()(const Point &left, const Point &right) {
    return IsHigher(left, right);
  }
};

} // namespace

LineStatus GetLineStatus(const Point &first, const Point &second) {
  if (first.x == second.x) {
    return LineStatus::Vertical;
  }
  return LineStatus::NonVertical;
}

double GetTangentCoefficent(const Point &first, const Point &second) {
  assert(GetLineStatus(first, second) == LineStatus::NonVertical);

  assert(first.y != second.y);

  return double(first.x - second.x) / double(first.y - second.y);
}

/// Мне очень стыдно
Triangle Triangle::SortedVertex() const {
  std::vector<Point> vertex = {a, b, c};
  std::sort(vertex.begin(), vertex.end(), HeightComparator());

  return Triangle{vertex[0], vertex[1], vertex[2]};
}

int32_t Triangle::MinimumHeight() const {
  return std::min(a.y, std::min(b.y, c.y));
}

int32_t Triangle::MaximumHeight() const {
  return std::max(a.y, std::max(b.y, c.y));
}
} // namespace geometry
} // namespace detail