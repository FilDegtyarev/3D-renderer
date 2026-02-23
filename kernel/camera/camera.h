#pragma once
#include "geometry/geometry.h"
#include "glm/mat4x4.hpp"
#include "types/types.h"
#include <glm/ext/matrix_float4x4.hpp>

namespace detail {
namespace camera {

class Camera {
public:
  Camera(HorizontalFOV horizontal_fov, AspectRatio aspect_ratio, NearPlaneDistance near_plane_distance, RenderDistance render_distance);

  M4 GetFrustumMatrix() const;
  M4 GetCameraMatrix() const;

  void Move(char c);
  void StopMoving(char c);

  void Rotate(char c);
  void StopRotating(char c);

  bool IsMoving() const;
  bool IsRotating() const;

  void UpdateView();
  void ResetPosition();
  bool IsReset() const;

  void ResetComplete();
  std::vector<geometry::Triangle> ClipTriangle(const geometry::Triangle &triangle) const;
  std::vector<geometry::Segment> ClipSegment(const geometry::Segment &segment) const;

  bool TestPoint(const geometry::Point &point) const;

private:
  struct Rotation {
    bool up = false;
    bool down = false;
    bool right = false;
    bool left = false;
  };

  struct Moving {
    bool toward = false;
    bool backward = false;
    bool right = false;
    bool left = false;
  };

  std::vector<geometry::Triangle> ClipTriangleWithPlane(const geometry::Triangle &triangle, const geometry::Plane &plane) const;
  std::vector<geometry::Segment> ClipSegmentWithPlane(const geometry::Segment &segment, const geometry::Plane &plane) const;

  double horizontal_fov;
  double aspect_ratio;
  double far_plane_distance;
  double focal_length;
  double near_plane_distance;

  double near_plane_y_top;
  double near_plane_y_bottom;
  double near_plane_x_right;
  double near_plane_x_left;

  M4 frusum_matrix;
  double speed_limit = 0.1;

  V3 eye_position;
  V3 gaze_direction;
  V3 view_up_direction;

  Rotation rotation;
  Moving moving;
  bool reset_position;
  std::vector<geometry::Plane> planes;
};

} // namespace camera
} // namespace detail
