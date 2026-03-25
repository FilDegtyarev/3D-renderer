#include "application/application.h"
#include "types/types.h"

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
      ThreadsCount{8},
      {"/Users/filipp/Documents/Models/Skull/12140_Skull_v3_L2.obj",
       "/Users/filipp/Documents/Models/Skull/Skull.jpg"},
      ScreenHeight{600}, ScreenWidth{800}, HorizontalFOV{90.0}, NearPlaneDistance{0.1},
      RenderDistance{100.0}
  );

  r_app.Show();
  return app.exec();

  // detail::parser::Parse(
  //     "/Users/filipp/Documents/Models/Skull/12140_Skull_v3_L2.obj",
  //     "/Users/filipp/Documents/Models/Skull/Skull.jpg"
  // );
  return 0;
}
