#include "geometry/geometry.h"
#include <vector>

namespace detail {
namespace world {
class LocalObject {

public:
  LocalObject(const std::vector<geometry::Triangle> &triangles);
  LocalObject(std::vector<geometry::Triangle> &&triangles);

  const std::vector<geometry::Triangle> &GetTriangles() const;

private:
  std::vector<geometry::Triangle> triangles;
};

} // namespace world

} // namespace detail
