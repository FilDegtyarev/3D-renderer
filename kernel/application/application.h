#pragma once
#include "QElapsedTimer.h"
#include "QWidget"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "light/light.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
#include "world/world.h"

#include <QKeyEvent>
#include <QTimer>
#include <qwidget.h>

namespace detail {

class ApplicationImpl : public QWidget {
  using Camera = camera::Camera;
  using World = world::World;
  using WorkerKeeper = concurrency::WorkerKeeper;
  using Screen = screen::Screen;
  using DirectionalLightSource = light::DirectionalLightSource;
  using Renderer = renderer::Renderer;
  Q_OBJECT
public:
  ApplicationImpl(
      int32_t threads_count, World&& world_, DirectionalLightSource&& light, Camera&& camera_,
      Height height, Width width, Height shadow_buffer_height, Width shadow_buffer_width
  );

  void ButtonPressed(int button);
  void ButtonReleased(int button);

  void DrawFrame(ForceScreenUpdate flag);
  void ConnectScreen(QVBoxLayout* layout);

  void StartRenderer();

  ~ApplicationImpl();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

private slots:
  void SceneTimer();

private:
  using Task = std::function<void(void)>;

  World world;

  DirectionalLightSource direction_light;
  Camera camera;
  Screen screen;
  Renderer renderer;

  QVBoxLayout* layout_;
  QTimer* timer_;
  QElapsedTimer* frame_drawing_timer_;
};

} // namespace detail

class Application {
  using Camera = detail::camera::Camera;
  using World = detail::world::World;
  using WorkerKeeper = detail::concurrency::WorkerKeeper;
  using Screen = detail::screen::Screen;
  using ApplicationImpl = detail::ApplicationImpl;
  using DirectionalLightSource = detail::light::DirectionalLightSource;

public:
  Application(
      ThreadsCount threads_count, std::vector<Model>&& models, DirectionalLightSource&& light,
      Height height, Width width, HorizontalFOV hf, NearPlaneDistance npd, RenderDistance rd,
      Height shadow_buffer_height, Width shadow_buffer_width
  );

  void Run();

private:
  std::unique_ptr<ApplicationImpl> impl;
};
