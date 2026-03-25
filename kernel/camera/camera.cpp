#include "camera.h"

#include "geometry/geometry.h"

#include <cmath>
#include <numbers>
namespace detail {
namespace camera {

static float PI = static_cast<float>(std::numbers::pi);
Camera::Camera(
    HorizontalFOV horizontal_fov_, AspectRatio aspect_ratio_,
    NearPlaneDistance near_plane_distance_, RenderDistance render_distance
) {

  horizontal_fov = horizontal_fov_();
  focal_length = 1.0f / tan((PI / 180.0f) * horizontal_fov / 2.0f);
  near_plane_distance = near_plane_distance_();

  aspect_ratio = aspect_ratio_();
  far_plane_distance = render_distance();

  near_plane_y_top = (near_plane_distance / focal_length) * aspect_ratio;
  near_plane_y_bottom = (near_plane_distance / focal_length) * -aspect_ratio;

  near_plane_x_left = -(near_plane_distance / focal_length);
  near_plane_x_right = (near_plane_distance / focal_length);

  eye_position = V3{0.f, 0.f, 0.f};
  gaze_direction = V3{0.f, 0.f, -1.f};
  view_up_direction = V3{0.f, 1.f, 0.f};

  rotation = static_cast<Rotating>(0);
  moving = static_cast<Moving>(0);
  planes = {};

  planes.emplace_back(V3{0, 0, -1}, -near_plane_distance);
  planes.emplace_back(V3{0, 0, 1}, far_plane_distance);

  float t1 = 1.0f / sqrt(focal_length * focal_length + 1);
  planes.emplace_back(V3{focal_length * t1, 0, -t1}, 0);
  planes.emplace_back(V3{-t1 * focal_length, 0, -t1}, 0);

  float t2 = 1.0f / sqrt(focal_length * focal_length + aspect_ratio * aspect_ratio);
  planes.emplace_back(V3{0, t2 * focal_length, -t2 * aspect_ratio}, 0);
  planes.emplace_back(V3{0, -t2 * focal_length, -t2 * aspect_ratio}, 0);
}

M4 Camera::GetFrustumMatrix() const {
  return geometry::GetFrustumMatrix(
      HorizontalFOV(horizontal_fov), AspectRatio(aspect_ratio),
      NearPlaneDistance{near_plane_distance}, RenderDistance(far_plane_distance),
      RightEdgeX(near_plane_x_right), LeftEdgeX(near_plane_x_left), TopEdgeY(near_plane_y_top),
      BottomEdgeY(near_plane_y_bottom)
  );
}

namespace {
inline M4 SetRow(int cid, const V3& vector, M4 matrix) {
  for (int i = 0; i < 3; ++i) {
    matrix[cid][i] = vector[i];
  }
  return matrix;
}
} // namespace

M4 Camera::GetCameraMatrix() const {
  // Shirley, 147
  V3 w = -gaze_direction / glm::length(gaze_direction);
  V3 tmp = glm::cross(view_up_direction, w);
  V3 u = tmp / glm::length(tmp);
  V3 v = glm::cross(w, u);

  M4 first = 0;
  first = SetRow(0, u, std::move(first));
  first = SetRow(1, v, std::move(first));
  first = SetRow(2, w, std::move(first));
  first[3][3] = 1;

  first = glm::transpose(first);

  M4 second = 1;
  second[0][3] = -eye_position.x;
  second[1][3] = -eye_position.y;
  second[2][3] = -eye_position.z;
  second = glm::transpose(second);
  return first * second;
}

void Camera::Move(Moving move) {
  moving |= move;
}

void Camera::StopMoving(Moving move) {
  moving ^= move;
}

void Camera::Rotate(Rotating rotating) {
  rotation |= rotating;
}

void Camera::StopRotating(Rotating rotating) {
  rotation ^= rotating;
}

bool Camera::IsMoving() const {
  return static_cast<uint8_t>(moving);
}

bool Camera::IsRotating() const {
  return static_cast<uint8_t>(rotation);
}

namespace {

inline M2 RotationMatrix(float angle) {
  M2 result = 0;
  float t = PI / 180.0 * angle;
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
    if ((moving & Moving::Toward) == true) {
      speed += gaze_direction / length(gaze_direction);
    } else if (static_cast<uint8_t>((moving & Moving::Backward))) {
      speed -= gaze_direction / length(gaze_direction);
    }

    if ((moving & Moving::Left) == true) {
      speed -= glm::cross(gaze_direction, view_up_direction) /
               glm::length(glm::cross(gaze_direction, view_up_direction));
    } else if ((moving & Moving::Right) == true) {
      speed += glm::cross(gaze_direction, view_up_direction) /
               glm::length(glm::cross(gaze_direction, view_up_direction));
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
    float standart = 1;
    if ((rotation & Rotating::Right) == true) {
      V3 normal = glm::cross(view_up_direction, gaze_direction);
      normal /= glm::length(normal);
      float t = PI / 180.0 * standart;
      gaze_direction = cosf(t) * gaze_direction + -sinf(t) * normal;
    }

    if ((rotation & Rotating::Left) == true) {
      V3 normal = glm::cross(view_up_direction, gaze_direction);
      normal /= glm::length(normal);
      float t = -PI / 180.0 * standart;
      gaze_direction = cosf(t) * gaze_direction + -sinf(t) * normal;
    }

    if ((rotation & Rotating::Up) == true) {
      float t = -PI / 180.0 * standart;
      V3 view_copy = view_up_direction;
      view_up_direction = -sinf(t) * gaze_direction + cosf(t) * view_copy;
      gaze_direction = cosf(t) * gaze_direction + sinf(t) * view_copy;
    }

    if ((rotation & Rotating::Down) == true) {
      float t = PI / 180.0 * standart;
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

bool Camera::IsReset() const {
  return reset_position;
}

void Camera::ResetComplete() {
  reset_position = false;
}

Camera::TriangleIntersected* Camera::ClipTriangle(
    const Triangle& triangle, TriangleIntersected* cur, TriangleIntersected* prev
) const {
  cur->Clear();
  prev->Clear();

  (*prev)[0] = triangle;
  prev->size = 1;

  for (const Plane& plane : planes) {
    for (int32_t triangle_index = 0; triangle_index < (*prev).size; ++triangle_index) {
      cur->Merge(ClipTriangleWithPlane((*prev)[triangle_index], plane));
    }

    std::swap(prev, cur);
    cur->Clear();
  }

  return prev;
}

std::vector<Camera::Segment> Camera::ClipSegment(const Segment& segment) const {
  Segment result = segment;
  for (const Plane& plane : planes) {
    if (ClipSegmentWithPlane(result, plane).empty()) {
      return {};
    } else {
      result = ClipSegmentWithPlane(result, plane)[0];
    }
  }
  return {result};
}

bool Camera::TestPoint(const geometry::Point& point) const {
  float result = 0;
  for (size_t i = 0; i < planes.size(); ++i) {
    result = std::min(result, planes[i](point));
  }

  return true;
}

geometry::TriangleIntersectedSingle
Camera::ClipTriangleWithPlane(const Triangle& triangle, const Plane& plane) const {
  return geometry::IntersectTriangleWithPlane(triangle, plane);
}

std::vector<Camera::Segment>
Camera::ClipSegmentWithPlane(const Segment& segment, const Plane& plane) const {
  float eps = 0;
  if (plane(segment.a) < -eps && plane(segment.b) < -eps) {
    return {};
  }

  return {geometry::IntersectSegmentWithPlane(segment, plane)};
}

} // namespace camera
} // namespace detail
