#pragma once
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"
#include <cfloat>
#include <vector>

template <class T, class Tag>
struct Type {
  explicit Type(const T &value) : value(value) {};
  T operator()() const { return value; }

private:
  T value;
};

using HorizontalFOV = Type<double, class horizontal_fov>;
using RenderDistance = Type<double, class render_distance>;
using AspectRatio = Type<double, class aspect_ratio>;

using RightEdgeX = Type<double, class right_edge_x>;
using LeftEdgeX = Type<double, class left_edge_x>;

using TopEdgeY = Type<double, class top_edge_y>;
using BottomEdgeY = Type<double, class bottom_edge_y>;

using M3 = glm::mat3x3;
using M4 = glm::mat4x4;

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  inline bool operator==(const Color &other) const = default;
};

struct ZColor {
  Color color;
  double z = DBL_MAX;
};

using ZBuffer = std::vector<std::vector<ZColor>>;
