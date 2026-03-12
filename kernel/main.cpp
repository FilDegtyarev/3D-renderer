#include "application/application.h"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
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
namespace detail {

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

  detail::screen::Screen screen =
      detail::screen::Screen(detail::screen::Height(600), detail::screen::Width(800));

  detail::world::LocalObject local =
      detail::parser::Parse("/Users/filipp/Documents/Models/cat.obj");

  local.Normalize(1);
  std::cout << "Model info:" << std::endl;
  std::cout << "Total Triangles: " << local.GetTriangles().size() << std::endl;
  detail::world::GlobalObject cube(std::move(local), glm::vec3{0, 0, -10}, detail::Eye());

  detail::world::WorldBuilder world_builder;
  world_builder.AddObject(std::move(cube));

  detail::world::World world = world_builder.Extract();

  detail::camera::Camera camera =
      detail::camera::Camera(HorizontalFOV{90.0}, AspectRatio{600.0 / 800.0},
                             NearPlaneDistance{0.1}, RenderDistance{100.0});

  Application r_app(8, std::move(world), std::move(camera), std::move(screen));

  r_app.show();
  // r_app.Run();
  return app.exec();
}
