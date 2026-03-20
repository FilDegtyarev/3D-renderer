#include "application/application.h"

#include "QDebug"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "parser/parser.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
#include "world/world.h"

#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qlogging.h>
#include <unordered_map>

namespace detail {
ApplicationImpl::ApplicationImpl(
    int32_t threads_count, world::World&& world_, camera::Camera&& camera_, screen::Screen&& screen_
)
    : QWidget(nullptr),
      threads_count(threads_count),
      world(std::move(world_)),
      camera(std::move(camera_)),
      screen(std::move(screen_)),
      renderer(
          renderer::Renderer(
              threads_count, screen.GetFlatScreen(),
              concurrency::WorkerKeeper(
                  threads_count, world.GetTrianglesCapacity(), world.GetSegmentCapacity(),
                  screen.GetScanlineCapacity(), screen.GetWidth(), screen.GetHeight(), camera, world
              )
          )
      ) {
  setFocusPolicy(Qt::StrongFocus);

  layout = new QVBoxLayout(this);

  ConnectScreen(layout);
  DrawFrame(ForceScreenUpdate{true});
  timer = std::make_unique<QTimer>();
  connect(timer.get(), SIGNAL(timeout()), this, SLOT(SceneTimer()));
  timer->start(0);
};

ApplicationImpl::~ApplicationImpl() {
  delete layout;
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

inline bool IsMovePressed(char c) {
  return c == 'w' || c == 'a' || c == 's' || c == 'd';
}

inline bool IsRotationPressed(char c) {
  return c == 'i' || c == 'j' || c == 'k' || c == 'l';
}

inline bool IsResetPressed(char c) {
  return c == ' ';
}
} // namespace

static std::unordered_map<char, camera::Rotating> rotating_mapping = {
    {'i', camera::Rotating::Up},
    {'k', camera::Rotating::Down},
    {'j', camera::Rotating::Left},
    {'l', camera::Rotating::Right}
};

static std::unordered_map<char, camera::Moving> moving_mapping = {
    {'w', camera::Moving::Toward},
    {'s', camera::Moving::Backward},
    {'a', camera::Moving::Left},
    {'d', camera::Moving::Right}
};

void ApplicationImpl::ButtonPressed(int button) {
  char c = DefineKey(button);
  if (IsMovePressed(c)) {
    camera.Move(moving_mapping[c]);
  } else if (IsRotationPressed(c)) {
    camera.Rotate(rotating_mapping[c]);
  } else if (IsResetPressed(c)) {
    camera.ResetPosition();
  }
}

void ApplicationImpl::ButtonReleased(int button) {
  char c = DefineKey(button);
  if (IsMovePressed(c)) {
    camera.StopMoving(moving_mapping[c]);
  } else if (IsRotationPressed(c)) {
    camera.StopRotating(rotating_mapping[c]);
  }
}

namespace {
inline M4 GenerateShiftMatrix(const V3& shift) {
  M4 matrix = 1;
  matrix[0][3] = shift.x;
  matrix[1][3] = shift.y;
  matrix[2][3] = shift.z;
  return matrix;
}

} // namespace

void ApplicationImpl::DrawFrame(ForceScreenUpdate flag) {
  if (camera.IsMoving() || camera.IsRotating() || camera.IsReset() || flag()) {
    if (camera.IsReset()) {
      camera.ResetComplete();
    }

    camera.UpdateView();

    renderer.Render();

    screen.Update();
  }
}

void detail::ApplicationImpl::ConnectScreen(QVBoxLayout* layout) {
  screen.Connect(layout);
}
} // namespace detail

void detail::ApplicationImpl::keyPressEvent(QKeyEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    ButtonPressed(event->key());
  }
}

void detail::ApplicationImpl::keyReleaseEvent(QKeyEvent* event) {
  if (event->type() == QEvent::KeyRelease) {
    ButtonReleased(event->key());
  }
}

void detail::ApplicationImpl::SceneTimer() {
  DrawFrame(ForceScreenUpdate{false});
  timer->start(0);
};

namespace {
detail::world::World CreateWorld(std::vector<std::string>&& models) {
  detail::world::WorldBuilder world_builder;
  for (auto& model : models) {
    detail::world::LocalObject local = detail::parser::Parse(model);
    local.Normalize(1);
    detail::world::GlobalObject model_global(std::move(local), glm::vec3{0, 0, -10}, 1);
    world_builder.AddObject(std::move(model_global));
  }

  detail::world::World world = world_builder.Extract();
  return world;
}
} // namespace

Application::Application(
    ThreadsCount threads_count, std::vector<std::string>&& models, ScreenHeight height,
    ScreenWidth width, HorizontalFOV hf, NearPlaneDistance npd, RenderDistance rd
)
    : impl(
          threads_count(), CreateWorld(std::move(models)),
          detail::camera::Camera(
              hf, AspectRatio{static_cast<float>(height()) / static_cast<float>(width())}, npd, rd
          ),
          detail::screen::Screen(height, width)
      ) {}

void Application::UpdateScreen() {
  std::cout << "called" << std::endl;
  impl.DrawFrame(ForceScreenUpdate{false});
}

void Application::Show() {
  impl.show();
}
