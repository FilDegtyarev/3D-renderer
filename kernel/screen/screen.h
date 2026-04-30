#pragma once
#include "types/types.h"

#include <QElapsedTimer>
#include <QImage>
#include <QVBoxLayout>
#include <cstdlib>
#include <qlabel.h>
#include <vector>

namespace detail {
namespace screen {

class Screen : public QWidget {
  Q_OBJECT
public:
  Screen(Height h, Width w);

  int32_t GetScanlineCapacity() const;

  int32_t GetHeight() const;
  int32_t GetWidth() const;

  void DrawFrame(const Frame& frame);

  void DrawFrameWithFps(const Frame& frame, float fps);

  float GetAspectRatio() const;

  ~Screen();

private:
  void Update(float fps);

  int32_t height;
  int32_t width;

  QVBoxLayout* layout_;
  QLabel* screen_;
  QLabel* fps_counter_;

  std::vector<QRgb> flat_screen;
};

} // namespace screen
} // namespace detail
