#pragma once
#include "geometry/geometry.h"
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
  using Point = geometry::Point;
  using Segment = geometry::Segment;
  using Triangle = geometry::Triangle;

  using Plane = geometry::Plane;
  using TriangleIntersected = geometry::TriangleIntersected;

public:
  Camera(
      HorizontalFOV horizontal_fov, AspectRatio aspect_ratio, NearPlaneDistance near_plane_distance,
      RenderDistance render_distance
  );

  M4 GetFrustumMatrix() const;
  M4 GetCameraMatrix() const;
  M3 GetNormalTransformMatrix() const;

  void Move(Moving move);
  void StopMoving(Moving move);

  void Rotate(Rotating rotating);
  void StopRotating(Rotating rotating);

  bool IsMoving() const;
  bool IsRotating() const;

  void UpdateCameraMatirx();
  void ResetPosition();
  bool IsReset() const;

  void Reset();

  TriangleIntersected* ClipTriangle(
      const Triangle& triangle, TriangleIntersected* first, TriangleIntersected* second,
      geometry::TriangleIntersectedSingle* buffer
  ) const;

  std::vector<Segment> ClipSegment(const Segment& segment) const;

  bool TestPoint(const Point& point) const;

  inline V3 GetCameraPosition() const { return camera_position; }

  inline const V3& GetGazeDirection() const { return gaze_direction; }

  inline bool IsClippingRequired(const Triangle& triangle) const {
    float eps = 1e-6;
    for (const auto& plane : planes) {
      if (plane(triangle.a) < eps || plane(triangle.b) < eps || plane(triangle.c) < eps) {
        return true;
      }
    }

    return false;
  }

private:
  using TriangleIntersectedSingle = geometry::TriangleIntersectedSingle;

  geometry::IntersectionStatus ClipTriangleWithPlane(
      const Triangle& triangle, const Plane& plane, TriangleIntersectedSingle* buffer
  ) const;

  std::vector<Segment> ClipSegmentWithPlane(const Segment& segment, const Plane& plane) const;

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
  float speed_limit = 0.1f;

  V3 camera_position;
  V3 gaze_direction;
  V3 view_up_direction;

  Rotating rotation;
  Moving moving;
  bool reset_position;
  std::vector<geometry::Plane> planes;
};

} // namespace camera
} // namespace detail
