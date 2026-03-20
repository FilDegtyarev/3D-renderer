#pragma once
#include "QWidget"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
#include "world/world.h"

#include <QKeyEvent>
#include <QTimer>
#include <memory>
#include <qwidget.h>

namespace detail {

class ApplicationImpl : public QWidget {
  Q_OBJECT
public:
  ApplicationImpl(
      int32_t threads_count, world::World&& world_, camera::Camera&& camera_,
      screen::Screen&& screen_
  );

  void ButtonPressed(int button);
  void ButtonReleased(int button);

  void DrawFrame(ForceScreenUpdate flag);
  void ConnectScreen(QVBoxLayout* layout);

  ~ApplicationImpl();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

private slots:
  void SceneTimer();

private:
  int32_t threads_count;
  world::World world;
  camera::Camera camera;
  screen::Screen screen;
  renderer::Renderer renderer;

  QVBoxLayout* layout;
  std::unique_ptr<QTimer> timer;
};

} // namespace detail

class Application {
public:
  Application(
      ThreadsCount threads_count, std::vector<std::string>&& models, ScreenHeight height,
      ScreenWidth width, HorizontalFOV hf, NearPlaneDistance npd, RenderDistance rd
  );

  void Show();

private:
  //  float MeasureFrameTime();

  void UpdateScreen();
  detail::ApplicationImpl impl;
};
