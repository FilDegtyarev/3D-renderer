#include "application/application.h"
#include "QDebug"
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qlogging.h>
#include <unordered_map>

namespace detail {
ApplicationImpl::ApplicationImpl(std::unique_ptr<world::World> &&world_, std::unique_ptr<camera::Camera> &&camera_, std::unique_ptr<screen::Screen> &&screen_,
                                 std::unique_ptr<renderer::Renderer> &&renderer_) {

  world = std::move(world_);
  camera = std::move(camera_);
  screen = std::move(screen_);
  renderer = std::move(renderer_);
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
    camera->Move(c);
  } else if (IsRotationPressed(c)) {
    camera->Rotate(c);
  } else if (IsResetPressed(c)) {
    camera->ResetPosition();
  }
}

void ApplicationImpl::ButtonReleased(int button) {
  char c = DefineKey(button);
  if (IsMovePressed(c)) {
    camera->StopMoving(c);
  } else if (IsRotationPressed(c)) {
    camera->StopRotating(c);
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

  if (camera->IsMoving() || camera->IsRotating() || camera->IsReset() || force) {
    if (camera->IsReset()) {
      camera->ResetComplete();
    }
    camera->UpdateView();
    screen->Update(renderer->Render(world, camera));
  }
}

void ApplicationImpl::ConnectScreen(QVBoxLayout *layout) { screen->Connect(layout); }
} // namespace detail

static double FPS = 120;
Application::Application(std::unique_ptr<detail::world::World> &&world, std::unique_ptr<detail::camera::Camera> &&camera, std::unique_ptr<detail::screen::Screen> &&screen,
                         std::unique_ptr<detail::renderer::Renderer> &&renderer)
    : QWidget(nullptr) {
  setFocusPolicy(Qt::StrongFocus);
  impl = std::make_unique<detail::ApplicationImpl>(std::move(world), std::move(camera), std::move(screen), std::move(renderer));
  layout = std::make_unique<QVBoxLayout>(this);
  impl->ConnectScreen(layout.get());
  impl->UpdateScreen(true);
  timer = std::make_unique<QTimer>();

  connect(timer.get(), SIGNAL(timeout()), this, SLOT(SceneTimer()));
  timer->start(1000.0 / FPS);
}

void Application::keyPressEvent(QKeyEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    impl->ButtonPressed(event->key());
  }
}

void Application::keyReleaseEvent(QKeyEvent *event) {
  if (event->type() == QEvent::KeyRelease) {
    impl->ButtonReleased(event->key());
  }
}

void Application::SceneTimer() {
  timer->start(1000.0 / FPS);
  UpdateScreen();
}

void Application::UpdateScreen() { impl->UpdateScreen(); }

