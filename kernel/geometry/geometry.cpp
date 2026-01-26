#include "geometry.h"
#include "types/types.h"
#include <algorithm>
#include <cmath>
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

M4 GetFrustumMatrix(HorizontalFOV horizontal_fov, AspectRatio aspect_ratio,
                    RenderDistance render_distance, RightEdgeX r, LeftEdgeX l,
                    TopEdgeY t, BottomEdgeY b) {
  M4 matrix;

  double near_plane_distance = 1 / tan(horizontal_fov() / 2.0);
  matrix[0][0] = 2.0 * near_plane_distance / (r() - l());
  matrix[0][1] = 0;
  matrix[0][2] = (r() + l()) / (r() - l()); // r + l == 0 ?
  matrix[0][3] = 0;

  matrix[1][0] = 0;
  matrix[1][1] = 2 * near_plane_distance / (t() - b());
  matrix[1][2] = (t() + b()) / (t() - b());
  matrix[1][3] = 0;

  matrix[2][0] = 0;
  matrix[2][1] = 0;
  matrix[2][2] = -(render_distance() + near_plane_distance) /
                 (render_distance() - near_plane_distance);
  matrix[2][3] = -2 * near_plane_distance * render_distance() /
                 (render_distance() - near_plane_distance);

  matrix[3][0] = 0;
  matrix[3][1] = 0;
  matrix[3][2] = -1;
  matrix[3][3] = 0;

  return matrix;
}
} // namespace geometry
} // namespace detail