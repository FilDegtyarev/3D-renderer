#pragma once
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"

#include <cfloat>
#include <cstdio>
#include <functional>
#include <vector>

template <class T, class Tag>
struct Type {
  explicit Type(const T& value) : value(value) {};
  T operator()() const { return value; }

private:
  T value;
};

using PathToObj = Type<std::string, class path_to_obj_proxy>;
using PathToTexture = Type<std::string, class path_to_texture_proxy>;

enum class BackFaceCullingStatus : uint8_t { Enabled, Disabled };

struct Model {
  PathToObj path_to_obj;
  PathToTexture path_to_texture;
  BackFaceCullingStatus bfc_status;
};

using Height = Type<int32_t, class screen_height_proxy>;
using Width = Type<int32_t, class screen_width_proxy>;

using ThreadsCount = Type<size_t, class threads_count_proxy>;

using HorizontalFOV = Type<float, class horizontal_fov_proxy>;
using RenderDistance = Type<float, class render_distance_proxy>;
using NearPlaneDistance = Type<float, class near_plane_distance_proxy>;
using AspectRatio = Type<float, class aspect_ratio_proxy>;

using RightEdgeX = Type<float, class right_edge_x_proxy>;
using LeftEdgeX = Type<float, class left_edge_x_proxy>;

using TopEdgeY = Type<float, class top_edge_y_proxy>;
using BottomEdgeY = Type<float, class bottom_edge_y_proxy>;

using ForceScreenUpdate = Type<bool, class force_screen_update_proxy>;

using M2 = glm::mat2x2;
using M3 = glm::mat3x3;
using M4 = glm::mat4x4;
using V4 = glm::vec4;
using V3 = glm::vec3;
using V2 = glm::vec2;

using Task = std::function<void(void)>;

struct Color {
  inline bool operator==(const Color& other) const = default;

  inline Color operator*(const float& value) const {
    return {
        static_cast<uint8_t>(red * value), static_cast<uint8_t>(green * value),
        static_cast<uint8_t>(blue * value)
    };
  }

  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

using Frame = std::vector<Color>;

struct ZColor {
  Color color = {0, 0, 0};
  float z = FLT_MAX;
};

struct VertexInfo {
  int32_t vertex_number;
  int32_t texture_number;
  int32_t normal_number = -1;
};

struct TextureCoordinates {
  float u;
  float v;

  inline TextureCoordinates operator+(const TextureCoordinates& other) const {
    return TextureCoordinates{u + other.u, v + other.v};
  }

  inline TextureCoordinates operator*(float other) const {
    return TextureCoordinates{u * other, v * other};
  }

  inline bool operator==(const TextureCoordinates& other) const = default;

  inline TextureCoordinates& operator=(const TextureCoordinates& other) = default;
};

class ZBuffer {
public:
  ZBuffer() = default;
  ZBuffer(Height height, Width width)
      : height(height()),
        width(width()),
        zbuffer(width() * height(), ZColor({0, 0, 0}, FLT_MAX)) {}

  inline ZColor& operator()(int32_t y, int32_t x) { return zbuffer[y * width + x]; }
  inline const ZColor& operator()(int32_t y, int32_t x) const { return zbuffer[y * width + x]; }

  inline int32_t Size() const { return height * width; }
  inline int32_t GetHeight() const { return height; }
  inline int32_t GetWidth() const { return width; }

private:
  int32_t height;
  int32_t width;
  std::vector<ZColor> zbuffer;
};

class ShadowMap {
public:
  ShadowMap() = default;
  ShadowMap(Height height, Width width)
      : height(height()),
        width(width()),
        depth_buffer(height() * width(), 100) {};

  inline void Clear() { std::fill(depth_buffer.begin(), depth_buffer.end(), 100); }

  inline const float operator()(int32_t y, int32_t x) const {
    if (y >= height || y < 0 || x < 0 || x >= width) {
      return 100;
    }

    return depth_buffer[y * width + x];
  }

  inline float& operator()(int32_t y, int32_t x) {
    if (y >= height || y < 0 || x < 0 || x >= width) {
      exit(666);
    }
    return depth_buffer[y * width + x];
  }

  inline int32_t Size() const { return width * height; }

  inline int32_t GetHeight() const { return height; }

  inline int32_t GetWidth() const { return width; }

private:
  int32_t width;
  int32_t height;

  std::vector<float> depth_buffer;
};

struct ShadowMapHelper {
  ShadowMap* shadow_map;
  const M4* frustum_to_world;
  const M4* view_transform_inv;
  const M4* world_to_light;
  float a_w, b_w, c_w;
};
