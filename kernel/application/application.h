#pragma once
#include "QWidget"
#include "camera/camera.h"
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
  ApplicationImpl(std::unique_ptr<world::World> &&world_, std::unique_ptr<camera::Camera> &&camera_, std::unique_ptr<screen::Screen> &&screen_, std::unique_ptr<renderer::Renderer> &&renderer_);

  void ButtonPressed(int button);
  void ButtonReleased(int button);

  void UpdateScreen(bool force = false);
  void ConnectScreen(QVBoxLayout *layout);

private:
  std::unique_ptr<world::World> world;
  std::unique_ptr<camera::Camera> camera;
  std::unique_ptr<screen::Screen> screen;
  std::unique_ptr<renderer::Renderer> renderer;
};

} // namespace detail

class Application : public QWidget {
  Q_OBJECT
public:
  Application(std::unique_ptr<detail::world::World> &&world, std::unique_ptr<detail::camera::Camera> &&camera, std::unique_ptr<detail::screen::Screen> &&screen,
              std::unique_ptr<detail::renderer::Renderer> &&renderer);

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private slots:
  void SceneTimer();

private:
  void UpdateScreen();
  std::unique_ptr<detail::ApplicationImpl> impl;
  std::unique_ptr<QVBoxLayout> layout;
  std::unique_ptr<QTimer> timer;
};
