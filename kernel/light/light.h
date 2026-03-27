#pragma once
#include "camera/camera.h"
#include "types/types.h"

#include <cstdio>

namespace detail {
namespace light {

class DirectionalLightSource {
  using Camera = camera::Camera;

public:
  DirectionalLightSource(const V3& base_light_direction);

  void UpdateDirection(const Camera& camera);

  inline Color CalculateColor(const V3& triangle_normal, const Color& color) const {
    // printf("%.6f\n", glm::dot(light_direction, triangle_normal));
    return color * std::max(-glm::dot(triangle_normal, current_light_direction), 0.f);
  }

private:
  V3 base_light_direction;
  V3 current_light_direction;
};

} // namespace light

} // namespace detail
