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
Screen::Screen(Height h, Width w) {
  height = h;
  width = w;
  screen = std::make_unique<QLabel>();
  flat_screen = std::vector<QRgb>(width * height);
};

int32_t Screen::GetScanlineCapacity() const { return height + width; }
int32_t Screen::GetHeight() const { return height; }
int32_t Screen::GetWidth() const { return width; }

void Screen::Connect(QVBoxLayout *layout) { layout->addWidget(screen.get()); }

// void Screen::Update() {

void Screen::Update() {
  QImage qimage(reinterpret_cast<uint8_t *>(flat_screen.data()), width, height,
                width * sizeof(uint32_t), QImage::Format_ARGB32);
  screen->setPixmap(QPixmap::fromImage(qimage));
}

float Screen::GetAspectRatio() const { return float(height) / float(width); }

std::vector<QRgb> &Screen::GetFlatScreen() { return flat_screen; }

} // namespace screen
} // namespace detail
