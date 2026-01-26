#pragma once
#include <QImage>
#include <QVBoxLayout>
#include <cstdlib>
#include <qlabel.h>

namespace detail {
namespace screen {
enum Height : int32_t;
enum Width : int32_t;

class Screen {
public:
  Screen(Height h, Width w);

  void Connect(QVBoxLayout *layout);
  void Update(const QImage &image);

private:
  int32_t height;
  int32_t width;
  std::unique_ptr<QLabel> screen;
};
} // namespace screen
} // namespace detail