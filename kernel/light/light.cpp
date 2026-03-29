#include "light.h"

namespace detail {
namespace light {

DirectionalLightSource::DirectionalLightSource(const V3& base_light_direction)
    : base_light_direction(base_light_direction) {};

void DirectionalLightSource::UpdateDirection(const Camera& camera) {
  V4 direcion(base_light_direction, 0.f);
  current_light_direction = glm::normalize(camera.GetCameraMatrix() * direcion);
}

} // namespace light
} // namespace detail
