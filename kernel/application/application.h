#pragma once
#include "QWidget"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "world/world.h"
#include <QKeyEvent>
#include <QTimer>
#include <memory>
#include <qwidget.h>

namespace detail {

class ApplicationImpl {
public:
  ApplicationImpl(int32_t threads_count, world::World &&world_, camera::Camera &&camera_,
                  screen::Screen &&screen_);

  void ButtonPressed(int button);
  void ButtonReleased(int button);

  void UpdateScreen(bool force = false);
  void ConnectScreen(QVBoxLayout *layout);

  void NextFrame();

private:
  int32_t threads_count;
  world::World world;
  camera::Camera camera;
  screen::Screen screen;
  renderer::Renderer renderer;
};

} // namespace detail

class Application : public QWidget {
  Q_OBJECT
public:
  Application(int32_t threads_count, detail::world::World &&world, detail::camera::Camera &&camera,
              detail::screen::Screen &&screen);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private slots:
  void SceneTimer();

private:
  //  double MeasureFrameTime();

  void UpdateScreen();
  detail::ApplicationImpl impl;
  std::unique_ptr<QVBoxLayout> layout;
  std::unique_ptr<QTimer> timer;
};
