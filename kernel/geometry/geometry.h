#pragma once
#include <qrgb.h>

namespace detail {
namespace geometry {

struct Point {
  int32_t x;
  int32_t y;
  double z;

  QRgb color;

  friend bool operator==(const Point &left, const Point &right) {
    return left.x == right.x && left.y == right.y && left.z == right.z &&
           left.color == right.color;
  }
};

enum LineStatus { Vertical, NonVertical };

LineStatus GetLineStatus(const Point &first, const Point &second);

double GetTangentCoefficent(const Point &first, const Point &second);

struct Triangle {
  Point a;
  Point b;
  Point c;

  Triangle SortedVertex() const;
  int32_t MinimumHeight() const;
  int32_t MaximumHeight() const;
};

} // namespace geometry

} // namespace detail