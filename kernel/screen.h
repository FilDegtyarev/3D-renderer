#pragma once
#include "geometry/geometry.h"

#include <QImage>
#include <QPixmap>

namespace detail {
QImage TestFigure();
QImage DrawLine(const detail::geometry::Point &from,
                const detail::geometry::Point &to);

geometry::Triangle RandomTriangle();

QImage GenerateTriangle();
} // namespace detail