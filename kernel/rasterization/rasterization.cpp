#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "screen/screen.h"
#include <iostream>
#include <qpoint.h>
namespace detail {

namespace rasterization {
const int32_t SIZE = 500;
void DrawLine(const detail::geometry::Point &from,
              const detail::geometry::Point &to, QImage &image) {
  std::vector<detail::geometry::Point> line =
      detail::rasterization::Bresenham(from, to);

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

  // return geometry::Triangle{geometry::Point{.x = 73, .y = 401},
  //                           geometry::Point{.x = 397, .y = 258},
  //                           geometry::Point{.x = 77, .y = 259}};

  return geometry::Triangle{RandomPoint(), RandomPoint(), RandomPoint()};
};

QImage GenerateRandomTriangleCarcass() {
  //
  QImage image(SIZE, SIZE, QImage::Format_RGB16);
  image.fill(Qt::black);
  geometry::Triangle triangle = RandomTriangle();

  DrawLine(triangle.a, triangle.b, image);
  DrawLine(triangle.b, triangle.c, image);
  DrawLine(triangle.a, triangle.c, image);

  return image;
}

namespace {
geometry::Triangle debug(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                         int32_t x3, int32_t y3) {
  return geometry::Triangle{geometry::Point{.x = x1, .y = y1},
                            geometry::Point{.x = x2, .y = y2},
                            geometry::Point{.x = x3, .y = y3}};
}
} // namespace
QImage GenerateRandomTrinagleFilled() {
  QImage image(SIZE, SIZE, QImage::Format_RGB16);
  image.fill(Qt::black);
  geometry::Triangle triangle = RandomTriangle();

  //   triangle = debug(0, 112, 328, 172, 100, 151);
  //   std::cout << triangle.a.x << " " << triangle.a.y << " " << triangle.b.x
  //   << " "
  //             << triangle.b.y << " " << triangle.c.x << " " << triangle.c.y
  //             << std::endl;

  for (size_t height = triangle.MinimumHeight();
       height <= triangle.MaximumHeight(); ++height) {
    std::vector<geometry::Point> scanline =
        rasterization::Scanline(triangle, height);

    for (const auto &pixel : scanline) {
      image.setPixel(QPoint(pixel.x, pixel.y), pixel.color);
    }
  }
  return image;
}

} // namespace rasterization
} // namespace detail