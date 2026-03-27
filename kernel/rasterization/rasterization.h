#pragma once
#include "geometry/geometry.h"
#include "light/light.h"
#include "textures/textures.h"
#include "types/types.h"

#include <vector>

namespace detail {
namespace rasterization {
using Point = geometry::Point;
using ScreenPoint = geometry::ScreenPoint;
using Triangle = geometry::Triangle;
using ScreenTriangle = geometry::ScreenTriangle;
using Segment = geometry::Segment;
using Texture = textures::Texture;
using DirectionalLightSource = light::DirectionalLightSource;

void DrawSegment(const Segment& segment, ZBuffer& zbuffer);

void DrawTriangle(
    const Triangle& triangle, ZBuffer& zbuffer, std::vector<ScreenPoint>& scanline_buffer,
    const Texture& texture, const DirectionalLightSource* direction_light
);

} // namespace rasterization

} // namespace detail
