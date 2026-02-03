#include "camera.h"
#include <cmath>
namespace detail {
namespace camera {

Camera::Camera(HorizontalFOV horizontal_fov_, AspectRatio aspect_ratio_,
               NearPlaneDistance near_plane_distance_,
               RenderDistance render_distance) {

  horizontal_fov = horizontal_fov_();
  focal_length = 1.0 / tan((3.1415926 / 180.0) * horizontal_fov / 2.0);
  near_plane_distance = near_plane_distance_();

  aspect_ratio = aspect_ratio_();
  far_plane_distance = render_distance();

  near_plane_y_top = (near_plane_distance / focal_length) * aspect_ratio;
  near_plane_y_bottom = (near_plane_distance / focal_length) * -aspect_ratio;

  near_plane_x_left = -(near_plane_distance / focal_length);
  near_plane_x_right = (near_plane_distance / focal_length);
}

M4 Camera::GetFrustumMatrix() const {
  return geometry::GetFrustumMatrix(
      HorizontalFOV(horizontal_fov), AspectRatio(aspect_ratio),
      NearPlaneDistance{near_plane_distance},
      RenderDistance(far_plane_distance), RightEdgeX(near_plane_x_right),
      LeftEdgeX(near_plane_x_left), TopEdgeY(near_plane_y_top),
      BottomEdgeY(near_plane_y_bottom));
}
} // namespace camera
} // namespace detail