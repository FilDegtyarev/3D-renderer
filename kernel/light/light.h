#pragma once
#include "camera/camera.h"
#include "glm/geometric.hpp"
#include "types/types.h"

#include <cassert>
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

  inline float CalculateDiffuseCoefficient(const V3& triangle_normal) const {
    return std::max(0.f, -glm::dot(triangle_normal, base_light_direction));
  }

private:
  V3 base_light_direction;
  V3 current_light_direction;
};

inline float
CalculateReflectionCoefficent(const V3& view_direction, const V3& light_reflected, float NS) {
  float angle = glm::dot(view_direction, light_reflected);
  if (angle > 0) {
    return std::pow(angle, NS);
  }
  return 0;
}

} // namespace light

} // namespace detail
