#include "parser/parser.h"

#include "geometry/geometry.h"
#include "textures/textures.h"
#include "types/types.h"
#include "world/object.h"

#include <QImage>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace detail {
namespace parser {

// Это просто ужасный код, ужасный файл но лучше я не сделаю
namespace {
std::vector<std::string> Split(const std::string& str) {
  std::vector<std::string> result;
  std::stringstream ss(str);
  std::string current;
  while (ss >> current) {
    result.push_back(current);
  }
  return result;
}

std::vector<VertexInfo> ParseFirstIndex(const std::string& face) {
  std::vector<VertexInfo> result;
  std::stringstream s(face);
  std::string vertex;

  s >> vertex;

  while (s >> vertex) {
    // t: v/t/n
    std::stringstream vertex_info(vertex);
    std::string number;
    std::vector<int32_t> tmp;
    while (std::getline(vertex_info, number, '/')) {
      tmp.push_back(std::stoi(number));
    }
    result.push_back({tmp[0] - 1, tmp[1] - 1, tmp[2]});
  }

  return result;
}

void Compress(geometry::Point& point, float compress) {
  float w = point.coordinates.w;
  point.coordinates *= 1.0f / compress;
  point.coordinates.w = w;
}

void Shift(geometry::Point& point, float shift) {
  point.coordinates.z -= shift;
}

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

void Rotate(geometry::Point& point) {
  point = point * RotateMatrix();
}

#include <fstream>
#include <string>
#include <vector>

std::vector<std::vector<Color>> LoadJpegWithQt(const std::string& filename) {
  QImage img(QString::fromStdString(filename));

  if (img.isNull()) {
    return {};
  }

  QImage rgbImg = img.convertToFormat(QImage::Format_RGB888);

  int width = rgbImg.width();
  int height = rgbImg.height();

  std::vector<std::vector<Color>> matrix(height, std::vector<Color>(width));

  for (int y = 0; y < height; ++y) {
    const uchar* line = rgbImg.scanLine(y);
    for (int x = 0; x < width; ++x) {
      matrix[y][x].red = line[x * 3 + 0];
      matrix[y][x].green = line[x * 3 + 1];
      matrix[y][x].blue = line[x * 3 + 2];
    }
  }

  return matrix;
}

} // namespace
world::LocalObject Parse(const std::string& filename, const std::string& texture_file) {
  std::ifstream fin(filename);
  std::ofstream fout("swaga.txt");
  textures::Texture texture;
  world::LocalObjectBuilder builder;
  std::vector<TextureCoordinates> coordinates;
  while (!fin.eof()) {
    std::string string;
    std::getline(fin, string);
    fout << string << std::endl;
    // continue;
    if (string[0] == 'v' && string[1] == ' ') {
      std::vector<std::string> result = Split(string);
      result.erase(result.begin());
      geometry::Point point;
      point.coordinates.x = std::stof(result[0]);
      point.coordinates.y = -std::stof(result[2]);
      point.coordinates.z = -std::stof(result[1]);
      point.color = {120, 120, 120};
      builder.AddVertex(point);
    } else if (string[0] == 'f') {
      std::vector<VertexInfo> vertexes = ParseFirstIndex(string);
      for (size_t i = 0; i < vertexes.size() - 2; ++i) {
        builder.AddTriangle({vertexes[0], vertexes[i + 1], vertexes[i + 2]});
      }
    } else if (string[0] == 'v' && string[1] == 't') {
      std::vector<std::string> c = Split(string);
      coordinates.push_back({std::stof(c[1]), std::stof(c[2])});
    }
  }

  fin.close();

  // Текстура
  std::vector<std::vector<Color>> colors = LoadJpegWithQt(texture_file);

  texture = textures::Texture(colors.size(), colors[0].size(), coordinates, colors);
  builder.AddTexture(texture);
  return builder.Extract();
}
} // namespace parser
} // namespace detail
