#pragma once
#include "types/types.h"
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

  int32_t GetScanlineCapacity() const;

  int32_t GetHeight() const;
  int32_t GetWidth() const;

  void Connect(QVBoxLayout *layout);
  void Update();

  double GetAspectRatio() const;

  std::vector<QRgb> &GetFlatScreen();

private:
  int32_t height;
  int32_t width;
  std::unique_ptr<QLabel> screen;
  std::vector<QRgb> flat_screen;
};

} // namespace screen
} // namespace detail
