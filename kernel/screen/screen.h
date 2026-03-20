#pragma once
#include "types/types.h"

#include <QElapsedTimer>
#include <QImage>
#include <QVBoxLayout>
#include <cstdlib>
#include <qlabel.h>

namespace detail {
namespace screen {

class Screen {
public:
  Screen(ScreenHeight h, ScreenWidth w);

  int32_t GetScanlineCapacity() const;

  int32_t GetHeight() const;
  int32_t GetWidth() const;

  void Connect(QVBoxLayout* layout);
  void Update();
  void UpdateFromZBuffer(const ZBuffer& zbuffer);

  float GetAspectRatio() const;

  std::vector<QRgb>& GetFlatScreen();

private:
  int32_t height;
  int32_t width;
  std::unique_ptr<QLabel> screen;
  std::vector<QRgb> flat_screen;
};

} // namespace screen
} // namespace detail
