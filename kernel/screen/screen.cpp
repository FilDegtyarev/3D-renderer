#include "screen.h"

#include "QPainter"
#include "types/types.h"

#include <cstdio>
#include <qboxlayout.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <string>

namespace detail {
namespace screen {
Screen::Screen(Height h, Width w)
    : QWidget(nullptr),
      height(h()),
      width(w()),
      layout_(new QVBoxLayout(this)),
      screen_(new QLabel(this)),
      fps_counter_(new QLabel(screen_)),
      flat_screen(width * height) {

  layout_->addWidget(screen_);
  fps_counter_->setStyleSheet("background: transparent; color: green; font-weight: italic;");
  fps_counter_->move(0, 0);
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

void Screen::DrawFrameWithFps(const Frame& frame, float fps) {
  for (size_t i = 0; i < flat_screen.size(); ++i) {
    flat_screen[i] = qRgb(frame[i].red, frame[i].green, frame[i].blue);
  }
  Update(fps);
}

float Screen::GetAspectRatio() const {
  return float(height) / float(width);
}

Screen::~Screen() {}

void Screen::Update(float fps) {
  QImage qimage(
      reinterpret_cast<uint8_t*>(flat_screen.data()), width, height, width * sizeof(uint32_t),
      QImage::Format_ARGB32
  );
  screen_->setPixmap(QPixmap::fromImage(qimage));
  fps_counter_->setText(QString::fromStdString("FPS: " + std::to_string(fps)));
}

} // namespace screen
} // namespace detail
