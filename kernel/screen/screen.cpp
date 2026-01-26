#include "screen.h"
#include <qboxlayout.h>

namespace detail {
namespace screen {
Screen::Screen(Height h, Width w) : height(h), width(w) {
  screen = std::make_unique<QLabel>();
};

void Screen::Connect(QVBoxLayout *layout) { layout->addWidget(screen.get()); }

void Screen::Update(const QImage &image) {
  screen->setPixmap(QPixmap::fromImage(image));
}

} // namespace screen
} // namespace detail