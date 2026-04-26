#include "application/application.h"
#include "exceptions/exceptions.h"
#include "light/light.h"
#include "types/types.h"

#include <QApplication>
#include <cstdio>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  try {
    Model skull(
        PathToObj{"models/skull/12140_Skull_v3_L2.obj"}, PathToTexture{"models/skull/Skull.jpg"},
        BackFaceCullingStatus::Enabled
    );

    Model cat(PathToObj{"models/cat.obj"}, PathToTexture{""}, BackFaceCullingStatus::Enabled);

    Model helmet(
        PathToObj{"models/helmet/Helmet.obj"}, PathToTexture{"models/helmet/Helmet_1.png"},
        BackFaceCullingStatus::Enabled
    );

    Model helmet_no_texture(
        PathToObj{"models/helmet/Helmet.obj"}, PathToTexture{""}, BackFaceCullingStatus::Disabled
    );

    detail::light::DirectionalLightSource light({0, 0, -1.0f});
    printf("Building the object\n");
    Application r_app(
        ThreadsCount{1}, {skull}, std::move(light), Height{720}, Width{1280}, HorizontalFOV{90.0},
        NearPlaneDistance{0.1}, RenderDistance{10.0}, Height{4096}, Width{4096}
    );

    r_app.Run();
    printf("[Executing]...\n");
    return app.exec();
  } catch (...) {
    detail::exceptions::react();
  }

  return 0;
}
