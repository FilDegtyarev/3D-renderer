#include "light.h"

namespace detail {
namespace light {

DirectionalLightSource::DirectionalLightSource(const V3& base_light_direction)
    : base_light_direction(base_light_direction) {};

void DirectionalLightSource::UpdateDirection(const Camera& camera) {
  V4 direcion(base_light_direction.x, base_light_direction.y, base_light_direction.z, 0.f);
  current_light_direction = direcion * camera.GetCameraMatrix() * camera.GetFrustumMatrix();
}

} // namespace light
} // namespace detail
