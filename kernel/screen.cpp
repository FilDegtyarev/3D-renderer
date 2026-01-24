#include "screen.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include <qpoint.h>

namespace detail {

using Pixel = detail::geometry::Point;
const int32_t SIZE = 500;
void DrawLine(const detail::geometry::Point &from,
              const detail::geometry::Point &to, QImage &image) {
  std::vector<Pixel> line = detail::rasterization::Bresenham(from, to);

  QRgb color = qRgb(255, 51, 153);

  for (const auto &pixel : line) {
    image.setPixel(QPoint(pixel.x, pixel.y), color);
  }
}

geometry::Point RandomPoint() {
  geometry::Point point;
  point.x = rand() % SIZE;
  point.y = rand() % SIZE;
  point.z = rand() % SIZE;
  return point;
}
geometry::Triangle RandomTriangle() {
  return geometry::Triangle{RandomPoint(), RandomPoint(), RandomPoint()};
};

QImage GenerateTriangle() {
  //
  QImage image(SIZE, SIZE, QImage::Format_RGB16);
  image.fill(Qt::black);
  geometry::Triangle triangle = RandomTriangle();

  DrawLine(triangle.a, triangle.b, image);
  DrawLine(triangle.b, triangle.c, image);
  DrawLine(triangle.a, triangle.c, image);

  return image;
}

} // namespace detail