#pragma once
#include "types/types.h"

#include <vector>
namespace detail {
namespace textures {

class Texture {
public:
  Texture() = default;
  Texture(
      int32_t height, int32_t width, const std::vector<TextureCoordinates>& coordinates,
      std::vector<std::vector<Color>> colors
  );

  inline bool IsActive() const { return colors.size() != 0; }

  inline const Color& operator()(float u, float v) const {
    int32_t h = std::min(int32_t(std::round((1.0f - v) * height)), height - 1);
    int32_t w = std::min(int32_t(std::round(u * width)), width - 1);

    h = std::max(0, h);
    w = std::max(0, w);

    return colors[h][w];
  }

  inline TextureCoordinates GetTextureCoordinates(int32_t number) const {
    return coordinates[number];
  }

private:
  int32_t height;
  int32_t width;

  std::vector<TextureCoordinates> coordinates;
  std::vector<std::vector<Color>> colors;

  std::vector<Material> materials;
};

} // namespace textures
} // namespace detail
