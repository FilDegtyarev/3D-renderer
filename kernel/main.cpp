#include "camera/camera.h"
#include "geometry/geometry.h"
#include "parser/parser.h"
#include "rasterization/rasterization.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "world/object.h"
#include "world/world.h"
#include <QApplication>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdlib>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <string>

#include "application/application.h"
namespace detail {
std::vector<geometry::Segment> generateCubeSegments(double distance) {

  geometry::Point v0 = {-1, 0, -distance};
  geometry::Point v1 = {1, 0, -distance};
  geometry::Point v2 = {-1, 1, -distance};
  geometry::Point v3 = {1, 1, -distance};

  geometry::Point v4 = {-1, 0, -distance - 1};
  geometry::Point v5 = {1, 0, -distance - 1};
  geometry::Point v6 = {-1, 1, -distance - 1};
  geometry::Point v7 = {1, 1, -distance - 1};

  return {{v0, v1}, {v1, v3}, {v2, v3}, {v0, v2}, {v4, v5}, {v5, v7}, {v7, v6}, {v4, v6}, {v0, v4}, {v1, v5}, {v3, v7}, {v2, v6}};
}

M3 Eye() {
  M3 matrix;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      matrix[i][j] = 0;
    }
  }
  matrix[0][0] = 1;
  matrix[1][1] = 1;
  matrix[2][2] = 1;
  return matrix;
}
} // namespace detail

int main(int argc, char *argv[]) {
  srand(1329);
  QApplication app(argc, argv);

  std::unique_ptr<detail::screen::Screen> screen = std::make_unique<detail::screen::Screen>(detail::screen::Height(600), detail::screen::Width(800));

  std::unique_ptr<detail::world::LocalObject> local = detail::parser::Parse("/Users/filipp/Documents/Models/cat.obj");

  local->Normalize(1);

  detail::world::GlobalObject cube(std::move(local), glm::vec3{0, 0, -10}, detail::Eye());

  detail::world::WorldBuilder world_builder;
  world_builder.AddObject(std::move(cube));

  std::unique_ptr<detail::world::World> world = world_builder.Extract();

  std::unique_ptr<detail::camera::Camera> camera = std::make_unique<detail::camera::Camera>(HorizontalFOV{90.0}, AspectRatio{600.0 / 800.0}, NearPlaneDistance{0.1}, RenderDistance{100.0});

  std::unique_ptr<detail::renderer::Renderer> renderer = std::make_unique<detail::renderer::Renderer>(ScreenHeight{600}, ScreenWidth{800});

  Application r_app(std::move(world), std::move(camera), std::move(screen), std::move(renderer));

  r_app.show();
  return app.exec();
}
