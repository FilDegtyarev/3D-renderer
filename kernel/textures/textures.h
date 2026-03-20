#pragma once
#include "types/types.h"

namespace detail {
namespace textures {

class Texture {
public:
  Texture() = default;
  Texture(int32_t height, int32_t width, std::vector<std::vector<Color>> colors);

  inline const Color &operator()(float u, float v) const {
    return colors[int32_t(u * height)][int32_t(v * width)];
  }

private:
  int32_t height;
  int32_t width;
  std::vector<std::vector<Color>> colors;
};

} // namespace textures
} // namespace detail

