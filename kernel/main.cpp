#include "application/application.h"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "parser/parser.h"
#include "rasterization/rasterization.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
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

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  Application r_app(
      ThreadsCount{8}, {"/Users/filipp/Documents/Models/cat.obj"}, ScreenHeight{600},
      ScreenWidth{800}, HorizontalFOV{90.0}, NearPlaneDistance{0.1}, RenderDistance{100.0}
  );

  r_app.Show();
  return app.exec();
}
