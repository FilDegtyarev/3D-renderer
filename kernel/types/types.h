#pragma once
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include <cfloat>
#include <iostream>
#include <memory>
#include <vector>

template <class T, class Tag>
struct Type {
  explicit Type(const T &value) : value(value) {};
  T operator()() const { return value; }

private:
  T value;
};

using ScreenHeight = Type<size_t, class screen_height_proxy>;
using ScreenWidth = Type<size_t, class screen_width_proxy>;

using HorizontalFOV = Type<float, class horizontal_fov_proxy>;
using RenderDistance = Type<float, class render_distance_proxy>;
using NearPlaneDistance = Type<float, class near_plane_distance_proxy>;
using AspectRatio = Type<float, class aspect_ratio_proxy>;

using RightEdgeX = Type<float, class right_edge_x_proxy>;
using LeftEdgeX = Type<float, class left_edge_x_proxy>;

using TopEdgeY = Type<float, class top_edge_y_proxy>;
using BottomEdgeY = Type<float, class bottom_edge_y_proxy>;

using M2 = glm::mat2x2;
using M3 = glm::mat3x3;
using M4 = glm::mat4x4;
using V4 = glm::vec4;
using V3 = glm::vec3;

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  inline bool operator==(const Color &other) const = default;
};

struct ZColor {
  Color color = {0, 0, 0};
  float z = FLT_MAX;
};

// using ZBuffer = std::vector<std::vector<ZColor>>;
class ZBuffer {
public:
  ZBuffer() = default;
  ZBuffer(int32_t height, int32_t width)
      : height(height), width(width), zbuffer(width * height, ZColor({0, 0, 0}, FLT_MAX)) {}

  inline ZColor &At(int32_t y, int32_t x) { return zbuffer[y * width + x]; }
  inline const ZColor &At(int32_t y, int32_t x) const { return zbuffer[y * width + x]; }

  inline int32_t Size() const { return height * width; }
  inline int32_t GetHeight() const { return height; }
  inline int32_t GetWidth() const { return width; }

private:
  int32_t height;
  int32_t width;
  std::vector<ZColor> zbuffer;
};
