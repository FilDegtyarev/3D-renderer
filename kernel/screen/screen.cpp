#include "screen.h"

#include "QPainter"
#include "qpaintdevice.h"
#include "types/types.h"

#include <qboxlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpoint.h>
namespace detail {
namespace screen {
Screen::Screen(ScreenHeight h, ScreenWidth w) {
  height = h();
  width = w();
  screen = std::make_unique<QLabel>();
  flat_screen = std::vector<QRgb>(width * height);
};

int32_t Screen::GetScanlineCapacity() const {
  return height + width;
}
int32_t Screen::GetHeight() const {
  return height;
}
int32_t Screen::GetWidth() const {
  return width;
}

void Screen::Connect(QVBoxLayout* layout) {
  layout->addWidget(screen.get());
}

void Screen::Update() {
  QImage qimage(
      reinterpret_cast<uint8_t*>(flat_screen.data()), width, height, width * sizeof(uint32_t),
      QImage::Format_ARGB32
  );
  screen->setPixmap(QPixmap::fromImage(qimage));
}

void Screen::UpdateFromZBuffer(const ZBuffer& zbuffer) {
  for (int32_t row_index = 0; row_index < height; row_index++) {
    for (int32_t element_index = 0; element_index < width; ++element_index) {
      Color c = zbuffer.At(row_index, element_index).color;
      flat_screen[row_index * width + element_index] = qRgb(c.red, c.green, c.blue);
    }
  }
  Update();
}

float Screen::GetAspectRatio() const {
  return float(height) / float(width);
}

std::vector<QRgb>& Screen::GetFlatScreen() {
  return flat_screen;
}

} // namespace screen
} // namespace detail
