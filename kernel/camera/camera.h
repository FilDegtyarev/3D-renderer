#include "geometry/geometry.h"
#include "glm/mat4x4.hpp"
#include "types/types.h"
#include <glm/ext/matrix_float4x4.hpp>

namespace detail {
namespace camera {

class Camera {
public:
  Camera(HorizontalFOV horizontal_fov, AspectRatio aspect_ratio,
         RenderDistance render_distance);

  M4 GetFrustumMatrix() const;

private:
  double horizontal_fov;
  double aspect_ratio;
  double far_plane_distance;
  double near_plane_distance;

  double near_plane_y_top;
  double near_plane_y_bottom;
  double near_plane_x_right;
  double near_plane_x_left;

  M4 frusum_matrix;
};

} // namespace camera
} // namespace detail