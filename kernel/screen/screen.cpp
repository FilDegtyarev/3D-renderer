#include "screen.h"
#include <qboxlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpoint.h>

namespace detail {
namespace screen {
Screen::Screen(Height h, Width w) {
  height = h;
  width = w;
  screen = std::make_unique<QLabel>();
};

void Screen::Connect(QVBoxLayout *layout) { layout->addWidget(screen.get()); }

void Screen::Update(const std::vector<std::vector<Color>> &image) {
  QImage qimage(width, height, QImage::Format_RGB16);
  qimage.fill(Qt::black);

  for (size_t i = 0; i < height; ++i) {
    for (size_t j = 0; j < width; ++j) {
      Color color = image[i][j];
      qimage.setPixel(QPoint(j, i), qRgb(color.red, color.green, color.blue));
    }
  }
  screen->setPixmap(QPixmap::fromImage(qimage));
}

double Screen::GetAspectRatio() const { return double(height) / double(width); }

} // namespace screen
} // namespace detail