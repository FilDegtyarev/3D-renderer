#pragma once
#include "geometry/geometry.h"

#include <QImage>
#include <QPixmap>

namespace detail {
namespace rasterization {
QImage TestFigure();
QImage DrawLine(const detail::geometry::Point &from,
                const detail::geometry::Point &to);

geometry::Triangle RandomTriangle();

QImage GenerateRandomTriangleCarcass();

QImage GenerateRandomTrinagleFilled();

} // namespace rasterization

} // namespace detail