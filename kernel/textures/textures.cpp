#include "textures/textures.h"
#include "types/types.h"

namespace detail {
namespace textures {
Texture::Texture(int32_t height, int32_t width, std::vector<std::vector<Color>> colors)
    : height(height), width(width), colors(colors) {};

} // namespace textures
} // namespace detail
