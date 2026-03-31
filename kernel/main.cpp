#include "application/application.h"
#include "exceptions/exceptions.h"
#include "light/light.h"
#include "types/types.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  try {
    Model skull(
        PathToObj{"/Users/filipp/Documents/Models/Skull/12140_Skull_v3_L2.obj"},
        PathToTexture{"/Users/filipp/Documents/Models/Skull/Skull.jpg"},
        BackFaceCullingStatus::Enabled
    );

    Model cat(
        PathToObj{"/Users/filipp/Documents/Models/cat.obj"}, PathToTexture{""},
        BackFaceCullingStatus::Enabled
    );

    detail::light::DirectionalLightSource light({0, 0, -1.0f});
    Application r_app(
        ThreadsCount{8}, {cat, skull}, std::move(light), Height{600}, Width{800},
        HorizontalFOV{90.0}, NearPlaneDistance{0.1}, RenderDistance{100.0}, Height{4096},
        Width{4096}
    );

    r_app.Run();
    return app.exec();
  } catch (...) {
    detail::exceptions::react();
  }
  return 0;
}
