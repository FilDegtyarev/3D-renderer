#include "geometry/geometry.h"
#include "rasterization/algorithm.h"
#include "screen/screen.h"
#include <iostream>
#include <qpoint.h>
#include <qrgb.h>
namespace detail {

namespace rasterization {

const int32_t SIZE = 500;
void DrawLine(const geometry::ScreenSegment &segment, ZBuffer &zbuffer) {
  geometry::ScreenPoint from = segment.a;
  geometry::ScreenPoint to = segment.b;

  std::vector<detail::geometry::ScreenPoint> line =
      detail::rasterization::Bresenham(from, to);

  for (const auto &pixel : line) {
    if (zbuffer.at(pixel.y).at(pixel.x).z > pixel.z) {
      zbuffer[pixel.y][pixel.x].z = pixel.z;

      zbuffer[pixel.y][pixel.x].color = pixel.color;
      zbuffer[pixel.y][pixel.x].color =
          Color{.red = 50, .blue = 100, .green = 150};
    }
  }
}

geometry::ScreenPoint RandomPoint() {
  geometry::ScreenPoint point;
  point.x = rand() % SIZE;
  point.y = rand() % SIZE;
  point.z = rand() % SIZE;
  return point;
}
geometry::ScreenTriangle RandomTriangle() {

  // return geometry::ScreenTriangle{geometry::ScreenPoint{.x = 73, .y = 401},
  //                           geometry::ScreenPoint{.x = 397, .y = 258},
  //                           geometry::ScreenPoint{.x = 77, .y = 259}};

  return geometry::ScreenTriangle{RandomPoint(), RandomPoint(), RandomPoint()};
};

// QImage GenerateRandomTriangleCarcass() {
//   //
//   QImage image(SIZE, SIZE, QImage::Format_RGB16);
//   image.fill(Qt::black);
//   geometry::ScreenTriangle triangle = RandomTriangle();

//   DrawLine(triangle.a, triangle.b, image);
//   DrawLine(triangle.b, triangle.c, image);
//   DrawLine(triangle.a, triangle.c, image);

//   return image;
// }

namespace {
geometry::ScreenTriangle debug(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                               int32_t x3, int32_t y3) {
  return geometry::ScreenTriangle{geometry::ScreenPoint{.x = x1, .y = y1},
                                  geometry::ScreenPoint{.x = x2, .y = y2},
                                  geometry::ScreenPoint{.x = x3, .y = y3}};
}
} // namespace
QImage GenerateRandomTrinagleFilled() {
  QImage image(SIZE, SIZE, QImage::Format_RGB16);
  image.fill(Qt::black);
  geometry::ScreenTriangle triangle = RandomTriangle();

  //   triangle = debug(0, 112, 328, 172, 100, 151);
  //   std::cout << triangle.a.x << " " << triangle.a.y << " " << triangle.b.x
  //   << " "
  //             << triangle.b.y << " " << triangle.c.x << " " << triangle.c.y
  //             << std::endl;

  for (size_t height = triangle.MinimumHeight();
       height <= triangle.MaximumHeight(); ++height) {
    std::vector<geometry::ScreenPoint> scanline =
        rasterization::Scanline(triangle, height);

    for (const auto &pixel : scanline) {
      Color color = pixel.color;
      image.setPixel(QPoint(pixel.x, pixel.y),
                     qRgb(color.red, color.green, color.blue));
    }
  }
  return image;
}

} // namespace rasterization
} // namespace detail