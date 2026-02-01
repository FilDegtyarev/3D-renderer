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

void Compress(geometry::Point &point, double compress) {

  point.x /= compress;
  point.y /= compress;
  point.z /= compress;
}

void Shift(geometry::Point &point, double shift) { point.z -= shift; }

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

  std::vector<geometry::Point> vertex;
  std::vector<geometry::Segment> segments;
  std::vector<geometry::Triangle> triangles;

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
      //   for (auto x : result) {
      //     std::cout << x << " ";
      //   }
      //   std::cout << std::endl;
      //   std::cout << "source: " << result[2] << " Before: " << point.z
      //             << " After: " << (point.z / 100) - 10 << std::endl;
      Compress(point, 1);
      Shift(point, 1);
      // Rotate(point);
      vertex.push_back(point);

      // std::cout << point.x << " " << point.y << " " << point.z << std::endl;
    } else if (string[0] == 'f') {
      std::vector<std::string> result = Split(string);
      result.erase(result.begin());
      if (result.size() == 2) {
        exit(666);
      } else {
        geometry::Point a = vertex[std::stoi(result[0]) - 1];
        geometry::Point b = vertex[std::stoi(result[1]) - 1];
        geometry::Point c = vertex[std::stoi(result[2]) - 1];

        segments.push_back({a, b});
        segments.push_back({b, c});
        segments.push_back({a, c});
        // triangles.push_back(triangle);
      }
    }
  }
  fin.close();
  return world::LocalObject(triangles, segments);
}
} // namespace parser
} // namespace detail