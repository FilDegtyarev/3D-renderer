#include "application/application.h"
#include "exceptions/exceptions.h"
#include "types/types.h"

#include <QApplication>
/*
int main() {
  QApplication qt_runtime(argc, argv);
  try {
    Application app;
    qt_runtime.exec();
  } catch(...) {
    except::react();
  }
  return 0;
}
*/
int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  try {
    Application r_app(
        ThreadsCount{8},
        {"/Users/filipp/Documents/Models/Skull/12140_Skull_v3_L2.obj",
         "/Users/filipp/Documents/Models/Skull/Skull.jpg"},
        Height{600}, Width{800}, HorizontalFOV{90.0}, NearPlaneDistance{0.1}, RenderDistance{100.0}
    );
    r_app.Run();
    return app.exec();
  } catch (...) {
    detail::exceptions::react();
  }

  // detail::parser::Parse(
  //     "/Users/filipp/Documents/Models/Skull/12140_Skull_v3_L2.obj",
  //     "/Users/filipp/Documents/Models/Skull/Skull.jpg"
  // );
  return 0;
}
