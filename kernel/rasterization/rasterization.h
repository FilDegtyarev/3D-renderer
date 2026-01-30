#pragma once
#include "geometry/geometry.h"
#include "types/types.h"

#include <QImage>
#include <QPixmap>

namespace detail {
namespace rasterization {
QImage TestFigure();
void DrawLine(const geometry::ScreenSegment &segment, ZBuffer &zbuffer);

geometry::ScreenTriangle RandomTriangle();

// QImage GenerateRandomTriangleCarcass();

QImage GenerateRandomTrinagleFilled();

} // namespace rasterization

} // namespace detail