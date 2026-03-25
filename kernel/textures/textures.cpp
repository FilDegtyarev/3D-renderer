#include "textures/textures.h"

#include "types/types.h"

namespace detail {
namespace textures {

Texture::Texture(
    int32_t height, int32_t width, const std::vector<TextureCoordinates>& coordinates,
    std::vector<std::vector<Color>> colors
)
    : height(height),
      width(width),
      coordinates(coordinates),
      colors(colors) {}
} // namespace textures
} // namespace detail
