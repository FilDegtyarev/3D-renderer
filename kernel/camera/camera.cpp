#include "camera.h"
#include <cmath>
namespace detail {
namespace camera {
Camera::Camera(HorizontalFOV horizontal_fov_, AspectRatio aspect_ratio_,
               RenderDistance render_distance) {

  horizontal_fov = horizontal_fov_();
  near_plane_distance = 1.0 / tan(horizontal_fov_() / 2.0);
  aspect_ratio = aspect_ratio_();
  far_plane_distance = render_distance();

  near_plane_y_top = aspect_ratio;
  near_plane_y_bottom = -aspect_ratio;

  near_plane_x_left = -1.0;
  near_plane_x_right = 1.0;
}

M4 Camera::GetFrustumMatrix() const {
  return geometry::GetFrustumMatrix(
      HorizontalFOV(horizontal_fov), AspectRatio(aspect_ratio),
      RenderDistance(far_plane_distance), RightEdgeX(near_plane_x_right),
      LeftEdgeX(near_plane_x_left), TopEdgeY(near_plane_y_top),
      BottomEdgeY(near_plane_y_bottom));
}
} // namespace camera
} // namespace detail