#pragma once
#include "geometry/geometry.h"
#include "glm/mat4x4.hpp"
#include "types/types.h"

#include <glm/ext/matrix_float4x4.hpp>

namespace detail {
namespace camera {
enum class Moving : unsigned char {
  Toward = 1 << 0,
  Backward = 1 << 1,
  Right = 1 << 2,
  Left = 1 << 3,
};

inline Moving& operator|=(Moving& left, Moving right) {
  left = static_cast<Moving>(static_cast<unsigned char>(left) | static_cast<unsigned char>(right));
  return left;
}

inline Moving operator|(Moving left, Moving right) {
  left |= right;
  return left;
}

inline Moving& operator^=(Moving& left, Moving right) {
  left = static_cast<Moving>(
      static_cast<uint8_t>(left) - (static_cast<uint8_t>(left) & static_cast<uint8_t>(right))
  );
  return left;
}

inline Moving operator^(Moving left, Moving right) {
  left ^= right;
  return left;
}

inline Moving operator&(Moving left, Moving right) {
  return static_cast<Moving>(static_cast<uint8_t>(left) & static_cast<uint8_t>(right));
}

inline bool operator==(const Moving& left, bool right) {
  return bool(static_cast<uint8_t>(left)) == right;
}

enum class Rotating : unsigned char {
  Up = 1 << 0,
  Down = 1 << 1,
  Right = 1 << 2,
  Left = 1 << 3,
};

inline Rotating& operator|=(Rotating& left, Rotating right) {
  left =
      static_cast<Rotating>(static_cast<unsigned char>(left) | static_cast<unsigned char>(right));
  return left;
}

inline Rotating operator|(Rotating left, Rotating right) {
  left |= right;
  return left;
}

inline Rotating& operator^=(Rotating& left, Rotating right) {
  left = static_cast<Rotating>(
      static_cast<uint8_t>(left) - (static_cast<uint8_t>(left) & static_cast<uint8_t>(right))
  );
  return left;
}

inline Rotating operator^(Rotating left, Rotating right) {
  left ^= right;
  return left;
}

inline Rotating operator&(Rotating left, Rotating right) {
  return static_cast<Rotating>(static_cast<uint8_t>(left) & static_cast<uint8_t>(right));
}

inline bool operator==(const Rotating& left, bool right) {
  return bool(static_cast<uint8_t>(left)) == right;
}
class Camera {
public:
  Camera(
      HorizontalFOV horizontal_fov, AspectRatio aspect_ratio, NearPlaneDistance near_plane_distance,
      RenderDistance render_distance
  );

  M4 GetFrustumMatrix() const;
  M4 GetCameraMatrix() const;

  void Move(Moving move);
  void StopMoving(Moving move);

  void Rotate(Rotating rotating);
  void StopRotating(Rotating rotating);

  bool IsMoving() const;
  bool IsRotating() const;

  void UpdateView();
  void ResetPosition();
  bool IsReset() const;

  void ResetComplete();
  geometry::TriangleIntersected ClipTriangle(const geometry::Triangle& triangle) const;
  std::vector<geometry::Segment> ClipSegment(const geometry::Segment& segment) const;

  bool TestPoint(const geometry::Point& point) const;

  inline V3 GetEyePosition() const { return eye_position; }

private:
  geometry::TriangleIntersectedSingle
  ClipTriangleWithPlane(const geometry::Triangle& triangle, const geometry::Plane& plane) const;

  std::vector<geometry::Segment>
  ClipSegmentWithPlane(const geometry::Segment& segment, const geometry::Plane& plane) const;
  float horizontal_fov;
  float aspect_ratio;
  float far_plane_distance;
  float focal_length;
  float near_plane_distance;

  float near_plane_y_top;
  float near_plane_y_bottom;
  float near_plane_x_right;
  float near_plane_x_left;

  M4 frusum_matrix;
  float speed_limit = 0.1;

  V3 eye_position;
  V3 gaze_direction;
  V3 view_up_direction;

  Rotating rotation;
  Moving moving;
  bool reset_position;
  std::vector<geometry::Plane> planes;
};

} // namespace camera
} // namespace detail
