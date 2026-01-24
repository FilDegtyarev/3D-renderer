#pragma once
#include <qrgb.h>

namespace detail {
namespace geometry {
struct Point {
  uint16_t x;
  uint16_t y;
  double z;

  QRgb color;

  friend bool operator==(const Point &left, const Point &right) {
    return left.x == right.x && left.y == right.y && left.z == right.z &&
           left.color == right.color;
  }
};

struct Triangle {
  Point a;
  Point b;
  Point c;
};

} // namespace geometry

} // namespace detail