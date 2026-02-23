#include "camera.h"
#include "geometry/geometry.h"
#include <cmath>
namespace detail {
namespace camera {

Camera::Camera(HorizontalFOV horizontal_fov_, AspectRatio aspect_ratio_, NearPlaneDistance near_plane_distance_, RenderDistance render_distance) {

  horizontal_fov = horizontal_fov_();
  focal_length = 1.0 / tan((3.1415926 / 180.0) * horizontal_fov / 2.0);
  near_plane_distance = near_plane_distance_();

  aspect_ratio = aspect_ratio_();
  far_plane_distance = render_distance();

  near_plane_y_top = (near_plane_distance / focal_length) * aspect_ratio;
  near_plane_y_bottom = (near_plane_distance / focal_length) * -aspect_ratio;

  near_plane_x_left = -(near_plane_distance / focal_length);
  near_plane_x_right = (near_plane_distance / focal_length);

  eye_position = V3{0, 0, 0};
  gaze_direction = V3{0, 0, -1};
  view_up_direction = V3{0, 1, 0};
  /*
  V3 eye_position;
  V3 gaze_direction;
  V3 view_up_direction;
  */
  rotation = {false, false, false, false};
  moving = {false, false, false, false};
  planes = {};

  planes.emplace_back(V3{0, 0, -1}, -near_plane_distance);
  planes.emplace_back(V3{0, 0, 1}, far_plane_distance);

  double t1 = 1.0 / sqrt(focal_length * focal_length + 1);
  planes.emplace_back(V3{focal_length * t1, 0, -t1}, 0);
  planes.emplace_back(V3{-t1 * focal_length, 0, -t1}, 0);

  double t2 = 1.0 / sqrt(focal_length * focal_length + aspect_ratio * aspect_ratio);
  planes.emplace_back(V3{0, t2 * focal_length, -t2 * aspect_ratio}, 0);
  planes.emplace_back(V3{0, -t2 * focal_length, -t2 * aspect_ratio}, 0);
}

M4 Camera::GetFrustumMatrix() const {
  return geometry::GetFrustumMatrix(HorizontalFOV(horizontal_fov), AspectRatio(aspect_ratio), NearPlaneDistance{near_plane_distance}, RenderDistance(far_plane_distance),
                                    RightEdgeX(near_plane_x_right), LeftEdgeX(near_plane_x_left), TopEdgeY(near_plane_y_top), BottomEdgeY(near_plane_y_bottom));
}

namespace {
inline void PutRow(int cid, const V3 &vector, M4 &matrix) {
  for (int i = 0; i < 3; ++i) {
    matrix[cid][i] = vector[i];
  }
}
} // namespace

M4 Camera::GetCameraMatrix() const {
  // Shirley, 147
  V3 w = -gaze_direction / glm::length(gaze_direction);
  V3 tmp = glm::cross(view_up_direction, w);
  V3 u = tmp / glm::length(tmp);
  V3 v = glm::cross(w, u);

  M4 first = 0;
  PutRow(0, u, first);
  PutRow(1, v, first);
  PutRow(2, w, first);
  first[3][3] = 1;

  first = glm::transpose(first);

  M4 second = 1;
  second[0][3] = -eye_position.x;
  second[1][3] = -eye_position.y;
  second[2][3] = -eye_position.z;
  second = glm::transpose(second);
  return first * second;
}

void Camera::Move(char c) {
  if (c == 'w') {
    moving.toward = true;
  } else if (c == 's') {
    moving.backward = true;
  } else if (c == 'a') {
    moving.left = true;
  } else if (c == 'd') {
    moving.right = true;
  }
}

void Camera::StopMoving(char c) {
  if (c == 'w') {
    moving.toward = false;
  } else if (c == 's') {
    moving.backward = false;
  } else if (c == 'a') {
    moving.left = false;
  } else if (c == 'd') {
    moving.right = false;
  }
}

void Camera::Rotate(char c) {
  if (c == 'j') {
    rotation.left = true;
  } else if (c == 'k') {
    rotation.down = true;
  } else if (c == 'l') {
    rotation.right = true;
  } else if (c == 'i') {
    rotation.up = true;
  }
}

void Camera::StopRotating(char c) {
  if (c == 'j') {
    rotation.left = false;
  } else if (c == 'k') {
    rotation.down = false;
  } else if (c == 'l') {
    rotation.right = false;
  } else if (c == 'i') {
    rotation.up = false;
  }
}

bool Camera::IsMoving() const { return moving.toward || moving.backward || moving.left || moving.right; }

bool Camera::IsRotating() const { return rotation.down || rotation.up || rotation.left || rotation.right; }

namespace {

inline M2 RotationMatrix(double angle) {
  M2 result = 0;
  double t = 3.1415926 / 180.0 * angle;
  result[0][0] = cos(t);
  result[1][1] = cos(t);
  result[0][1] = sin(t);
  result[1][0] = -sin(t);
  return glm::transpose(result);
}

} // namespace

void Camera::UpdateView() {
  if (IsMoving()) {
    V3 speed = {0, 0, 0};
    if (moving.toward == true) {
      speed += gaze_direction / length(gaze_direction);
    } else if (moving.backward == true) {
      speed -= gaze_direction / length(gaze_direction);
    }

    if (moving.left == true) {
      speed -= glm::cross(gaze_direction, view_up_direction) / glm::length(glm::cross(gaze_direction, view_up_direction));
    } else if (moving.right == true) {
      speed += glm::cross(gaze_direction, view_up_direction) / glm::length(glm::cross(gaze_direction, view_up_direction));
    }
    if (speed == V3{0, 0, 0}) {
      return;
    }

    if (glm::length(speed) == 0) {
      assert(false);
    }

    speed = speed / glm::length(speed) * float(speed_limit);
    eye_position += speed;
  }

  if (IsRotating()) {
    double standart = 1;
    if (rotation.right) {
      V3 normal = glm::cross(view_up_direction, gaze_direction);
      normal /= glm::length(normal);
      float t = 3.1415926 / 180.0 * standart;
      gaze_direction = cosf(t) * gaze_direction + -sinf(t) * normal;
    }

    if (rotation.left) {
      V3 normal = glm::cross(view_up_direction, gaze_direction);
      normal /= glm::length(normal);
      float t = -3.1415926 / 180.0 * standart;
      gaze_direction = cosf(t) * gaze_direction + -sinf(t) * normal;
    }

    if (rotation.up) {
      float t = -3.1415926 / 180.0 * standart;
      V3 view_copy = view_up_direction;
      view_up_direction = -sinf(t) * gaze_direction + cosf(t) * view_copy;
      gaze_direction = cosf(t) * gaze_direction + sinf(t) * view_copy;
    }

    if (rotation.down) {
      float t = 3.1415926 / 180.0 * standart;
      V3 view_copy = view_up_direction;
      view_up_direction = -sinf(t) * gaze_direction + cosf(t) * view_copy;
      gaze_direction = cosf(t) * gaze_direction + sinf(t) * view_copy;
    }
  }
}

void Camera::ResetPosition() {
  eye_position = V3{0, 0, 0};
  gaze_direction = V3{0, 0, -1};
  view_up_direction = V3{0, 1, 0};
  reset_position = true;
}

bool Camera::IsReset() const { return reset_position; }

void Camera::ResetComplete() { reset_position = false; }

std::vector<geometry::Triangle> Camera::ClipTriangle(const geometry::Triangle &triangle) const {
  std::vector<geometry::Triangle> clipped = {};
  std::vector<geometry::Triangle> buffer = {triangle};
  for (const geometry::Plane &plane : planes) {
    for (const geometry::Triangle &t : buffer) {
      for (const geometry::Triangle &t_clipped : ClipTriangleWithPlane(t, plane)) {
        clipped.push_back(t_clipped);
      }
    }
    buffer = clipped;
    clipped.clear();
  }

  return buffer;
}

std::vector<geometry::Triangle> Camera::ClipTriangleWithPlane(const geometry::Triangle &triangle, const geometry::Plane &plane) const { return geometry::IntersectTriangleWithPlane(triangle, plane); }

std::vector<geometry::Segment> Camera::ClipSegment(const geometry::Segment &segment) const {
  // Пересечь со всеми плоскостями
  geometry::Segment result = segment;
  for (const geometry::Plane &plane : planes) {
    if (ClipSegmentWithPlane(result, plane).empty()) {
      return {};
    } else {
      result = ClipSegmentWithPlane(result, plane)[0];
    }
  }
  return {result};
}

std::vector<geometry::Segment> Camera::ClipSegmentWithPlane(const geometry::Segment &segment, const geometry::Plane &plane) const {
  double eps = 0;
  if (plane(segment.a) < -eps && plane(segment.b) < -eps) {
    return {};
  }

  return {geometry::IntersectSegmentWithPlane(segment, plane)};
}

bool Camera::TestPoint(const geometry::Point &point) const {
  double result = 0;
  for (size_t i = 0; i < planes.size(); ++i) {
    result = std::min(result, planes[i](point));
  }

  return true;
}

} // namespace camera
} // namespace detail
