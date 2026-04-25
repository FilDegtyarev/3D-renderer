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

  const V3& GetBaseDirection() const;

  inline Color CalculateColor(const V3& triangle_normal, const Color& color) const {
    // printf("%.6f\n", glm::dot(light_direction, triangle_normal));
    // float value = std::max(0.f, -glm::dot(triangle_normal, current_light_direction));
    float value = std::max(0.f, -glm::dot(triangle_normal, base_light_direction));
    return color * value;
  }

private:
  V3 base_light_direction;
  V3 current_light_direction;
};

} // namespace light

} // namespace detail
