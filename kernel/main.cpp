#include "camera/camera.h"
#include "geometry/geometry.h"
#include "rasterization/rasterization.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
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

  return {{v0, v1}, {v1, v3}, {v2, v3}, {v0, v2}, {v4, v5}, {v5, v7},
          {v7, v6}, {v4, v6}, {v0, v4}, {v1, v5}, {v3, v7}, {v2, v6}};

  // {v4, v5}, {v5, v6}, {v6, v7}, {v7, v4},

  // {v0, v4}, {v1, v5}, {v2, v6}, {v3, v7}};
}

} // namespace detail

int main(int argc, char *argv[]) {
  srand(1329);
  QApplication app(argc, argv);

  QWidget window;
  window.setWindowTitle("Треугольники");

  auto *layout = new QVBoxLayout(&window);

  detail::screen::Screen screen(detail::screen::Height(600),
                                detail::screen::Width(800));
  screen.Connect(layout);
  detail::world::LocalObject cube_local({},
                                        detail::generateCubeSegments(100.0));

  detail::world::GlobalObject cube(cube_local, glm::vec3(), M3());

  detail::world::World world(std::vector<detail::world::GlobalObject>{cube});

  detail::camera::Camera camera(HorizontalFOV{90.0}, AspectRatio{600.0 / 800.0},
                                RenderDistance{1000.0});

  detail::renderer::Renderer renderer(ScreenHeight{600}, ScreenWidth{800});

  screen.Update(renderer.Render(world, camera));
  //  auto *button = new QPushButton("еще треугольники");

  // layout->addWidget(button);

  // QObject::connect(button, &QPushButton::clicked, [&]() {
  //   QImage img = detail::rasterization::GenerateRandomTrinagleFilled();
  //   // imageLabel->setPixmap(QPixmap::fromImage(img));
  //   // screen.Update(image);
  //   screen.Update(renderer.Render(world, camera));
  // });

  window.show();

  return app.exec();
}
