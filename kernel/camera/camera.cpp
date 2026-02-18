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

  double t1 = focal_length / sqrt(focal_length * focal_length + 1);
  planes.emplace_back(V3{t1, 0, -t1}, 0);
  planes.emplace_back(V3{-t1, 0, -t1}, 0);

  double t2 = focal_length / sqrt(focal_length * focal_length + aspect_ratio * aspect_ratio);
  planes.emplace_back(V3{0, t2, -t2}, 0);
  planes.emplace_back(V3{0, -t2, -t2}, 0);
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

  // КАЖЕТСЯ
  first = glm::transpose(first);

  M4 second = 1;
  second[0][3] = -eye_position.x;
  second[1][3] = -eye_position.y;
  second[2][3] = -eye_position.z;
  // for (int i = 0; i < 4; ++i) {
  //   for (int j = 0; j < 4; ++j) {
  //     std::cout << second[i][j] << "\t";
  //   }
  //   std::cout << '\n';
  // }
  // std::cout << std::endl;
  // КАЖЕСЯ12
  second = glm::transpose(second);
  return first * second;
}

void Camera::Move(char c) {
  if (c == 'w') {
    // Вперед
    moving.toward = true;
  } else if (c == 's') {
    moving.backward = true;
  } else if (c == 'a') {
    moving.left = true;
  } else if (c == 'd') {
    moving.right = true;
  }

  // if (speed.x == 0 && direction.x != 0) {
  //   // std::cout << "moving x" << std::endl;
  //   speed.x = direction.x;
  // } else if (speed.y == 0 && direction.y != 0) {
  //   // std::cout << "moving y" << std::endl;
  //   speed.y = direction.y;
  // } else if (speed.z == 0 && direction.z != 0) {
  //   // std::cout << "moving z" << std::endl;
  //   speed.z = direction.z;
  // }
  // eye_position += direction;
  // Tell(speed);
}

void Camera::StopMoving(char c) {
  if (c == 'w') {
    // Вперед
    moving.toward = false;
  } else if (c == 's') {
    moving.backward = false;
  } else if (c == 'a') {
    moving.left = false;
  } else if (c == 'd') {
    moving.right = false;
  }

  // if (speed.x != 0 && direction.x != 0) {
  //   speed.x = 0;
  //   // std::cout << "stop moving x" << std::endl;
  // } else if (speed.y != 0 && direction.y != 0) {
  //   // std::cout << "stop moving y" << std::endl;
  //   speed.y = 0;
  // } else if (speed.z != 0 && direction.z != 0) {
  //   // std::cout << "stop moving z" << std::endl;
  //   speed.z = 0;
  // }
  // Tell(speed);
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
M3 RotationMatrix(double angle) {
  M3 result = 0;
  double t = 3.1415926 / 180.0 * angle;
  result[0][0] = cos(t);
  result[0][1] = 0;
  result[0][2] = sin(t);
  result[1][0] = 0;
  result[1][1] = 1;
  result[1][2] = 0;
  result[2][0] = -sin(t);
  result[2][1] = 0;
  result[2][2] = cos(t);
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
    // std::cout << eye_position.x << " " << eye_position.y << " " << eye_position.z << std::endl;
  }

  if (IsRotating()) {
    double standart = 1;
    if (rotation.right) {
      // Тихо
      M3 r = RotationMatrix(-standart);
      gaze_direction = r * gaze_direction;
    }

    if (rotation.left) {
      M3 r = RotationMatrix(standart);
      gaze_direction = r * gaze_direction;
    }
  }
}

std::vector<geometry::Triangle> Camera::ClipTriangle(const geometry::Triangle &triangle) const { return {triangle}; }

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

} // namespace camera
} // namespace detail
