#include "parser/parser.h"
#include "geometry/geometry.h"
#include "world/object.h"
#include <fstream>
#include <sstream>

namespace detail {
namespace parser {

namespace {
std::vector<std::string> Split(const std::string &str) {
  std::vector<std::string> result;
  std::stringstream ss(str);
  std::string current;
  while (std::getline(ss, current, ' ')) {
    result.push_back(current);
  }
  return result;
}

void Compress(geometry::Point &point, float compress) {

  point.x /= compress;
  point.y /= compress;
  point.z /= compress;
}

void Shift(geometry::Point &point, float shift) { point.z -= shift; }

M3 RotateMatrix() {
  M3 matrix;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      matrix[i][j] = 0;
    }
  }
  matrix[0][2] = 1;
  matrix[1][1] = 1;
  matrix[2][0] = -1;

  return matrix;
}

void Rotate(geometry::Point &point) { point = point * RotateMatrix(); }

} // namespace
world::LocalObject Parse(const std::string &filename) {
  std::ifstream fin(filename);

  world::LocalObjectBuilder builder;

  while (!fin.eof()) {
    std::string string;
    std::getline(fin, string);
    if (string[0] == 'v') {
      std::vector<std::string> result = Split(string);
      result.erase(result.begin());
      geometry::Point point;
      point.x = std::stod(result[0]);
      point.y = -std::stod(result[2]);
      point.z = -std::stod(result[1]);
      point.color = {255, 255, 255};
      builder.AddVertex(point);
    } else if (string[0] == 'f') {
      std::vector<std::string> result = Split(string);
      result.erase(result.begin());
      if (result.size() == 2) {
        exit(666);
      } else {
        int32_t a = std::stoi(result[0]) - 1;
        int32_t b = std::stoi(result[1]) - 1;
        int32_t c = std::stoi(result[2]) - 1;
        builder.AddTriangle({a, b, c});
      }
    }
  }

  fin.close();
  return builder.Extract();
}
} // namespace parser
} // namespace detail
