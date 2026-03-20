#include "application/application.h"
#include "QDebug"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qlogging.h>

namespace detail {
ApplicationImpl::ApplicationImpl(int32_t threads_count, world::World &&world_,
                                 camera::Camera &&camera_, screen::Screen &&screen_)
    : threads_count(threads_count), world(std::move(world_)), camera(std::move(camera_)),
      screen(std::move(screen_)),
      renderer(renderer::Renderer(
          threads_count, screen.GetFlatScreen(),
          concurrency::WorkerKeeper(threads_count, world.GetTrianglesCapacity(),
                                    world.GetSegmentCapacity(), screen.GetScanlineCapacity(),
                                    screen.GetWidth(), screen.GetHeight(), camera, world))) {
        // std::cout << "[application impl]: constructed\n" << std::endl;
      };

void ApplicationImpl::NextFrame() {
  QCoreApplication::processEvents();
  renderer.FrameSucceed();
}

namespace {

char DefineKey(int button) {
  char c = 0;
  if (button == Qt::Key_W) {
    c = 'w';
  } else if (button == Qt::Key_A) {
    c = 'a';
  } else if (button == Qt::Key_S) {
    c = 's';
  } else if (button == Qt::Key_D) {
    c = 'd';
  } else if (button == Qt::Key_J) {
    c = 'j';
  } else if (button == Qt::Key_K) {
    c = 'k';
  } else if (button == Qt::Key_L) {
    c = 'l';
  } else if (button == Qt::Key_I) {
    c = 'i';
  } else if (button == Qt::Key_Space) {
    c = ' ';
  } else {
    c = 0;
  }
  return c;
}

inline V3 Speed(char c) {
  if (c == 'w') {
    return {0, 0, -1};
  } else if (c == 'd') {
    return {1, 0, 0};
  } else if (c == 'a') {
    return {-1, 0, 0};
  } else {
    return {0, 0, 1};
  }
}

inline bool IsMovePressed(char c) { return c == 'w' || c == 'a' || c == 's' || c == 'd'; }

inline bool IsRotationPressed(char c) { return c == 'i' || c == 'j' || c == 'k' || c == 'l'; }

inline bool IsResetPressed(char c) { return c == ' '; }
} // namespace

void ApplicationImpl::ButtonPressed(int button) {
  char c = DefineKey(button);
  if (IsMovePressed(c)) {
    camera.Move(c);
  } else if (IsRotationPressed(c)) {
    camera.Rotate(c);
  } else if (IsResetPressed(c)) {
    camera.ResetPosition();
  }
}

void ApplicationImpl::ButtonReleased(int button) {
  char c = DefineKey(button);
  if (IsMovePressed(c)) {
    camera.StopMoving(c);
  } else if (IsRotationPressed(c)) {
    camera.StopRotating(c);
  }
}

namespace {
inline M4 GenerateShiftMatrix(const V3 &shift) {
  M4 matrix = 1;
  matrix[0][3] = shift.x;
  matrix[1][3] = shift.y;
  matrix[2][3] = shift.z;
  return matrix;
}

} // namespace

void ApplicationImpl::UpdateScreen(bool force) {

  if (camera.IsMoving() || camera.IsRotating() || camera.IsReset() || force) {
    if (camera.IsReset()) {
      camera.ResetComplete();
    }
    camera.UpdateView();
    // вот тут вот начинается многопоточка
    // std::cout << "Clearly new frame\n" << std::endl;
    renderer.Render();
    NextFrame();
    screen.Update();
    // QCoreApplication::processEvents();
  }
}

void ApplicationImpl::ConnectScreen(QVBoxLayout *layout) { screen.Connect(layout); }
} // namespace detail

static float FPS = 60;
Application::Application(int32_t threads_count, detail::world::World &&world,
                         detail::camera::Camera &&camera, detail::screen::Screen &&screen)
    : QWidget(nullptr),
      impl(threads_count, std::move(world), std::move(camera), std::move(screen)) {

  setFocusPolicy(Qt::StrongFocus);
  layout = std::make_unique<QVBoxLayout>(this);
  impl.ConnectScreen(layout.get());
  impl.UpdateScreen(true);
  timer = std::make_unique<QTimer>();
  std::cout << "Start" << std::endl;
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(SceneTimer()));
  timer->start(20);
  // timer->start(1000.0 / FPS);
  // std::cout << "[application]: new frame is proceeding..." << std::endl;
  // SceneTimer();
}

// void Application::Run() {

// }

void Application::keyPressEvent(QKeyEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    impl.ButtonPressed(event->key());
  }
}

void Application::keyReleaseEvent(QKeyEvent *event) {
  if (event->type() == QEvent::KeyRelease) {
    impl.ButtonReleased(event->key());
  }
}

void Application::SceneTimer() {
  UpdateScreen();
  timer->start(0);
};

void Application::UpdateScreen() { impl.UpdateScreen(); }

