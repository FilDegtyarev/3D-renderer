#include "application/application.h"

#include "QDebug"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "geometry/geometry.h"
#include "parser/parser.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
#include "world/world.h"

#include <algorithm>
#include <functional>
#include <iostream>
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
      renderer(threads_count, screen.GetHeight(), screen.GetWidth(), MakeWorkerKeeper()) {

  setFocusPolicy(Qt::StrongFocus);
  layout = new QVBoxLayout(this);

  ConnectScreen(layout);
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(SceneTimer()));
}

void ApplicationImpl::StartRenderer() {
  renderer.UnleashWorkers(&camera, &world);
  DrawFrame(ForceScreenUpdate{true});
  timer->start(0);
}

ApplicationImpl::~ApplicationImpl() {
  delete layout;
  delete timer;
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

void ApplicationImpl::DrawFrame(ForceScreenUpdate flag) {
  if (camera.IsMoving() || camera.IsRotating() || camera.IsReset() || flag()) {
    if (camera.IsReset()) {
      camera.ResetComplete();
    }

    camera.UpdateView();

    const Frame& new_frame = renderer.MakeFrame();

    screen.DrawFrame(new_frame);
  }
}

void ApplicationImpl::ConnectScreen(QVBoxLayout* layout) {
  screen.Connect(layout);
}

concurrency::WorkerKeeper ApplicationImpl::MakeWorkerKeeper() const {
  return WorkerKeeper(
      threads_count, world.GetTrianglesCapacity(), world.GetSegmentCapacity(),
      screen.GetScanlineCapacity(), screen.GetWidth(), screen.GetHeight()
  );
}

void ApplicationImpl::keyPressEvent(QKeyEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    ButtonPressed(event->key());
  }
}

void ApplicationImpl::keyReleaseEvent(QKeyEvent* event) {
  if (event->type() == QEvent::KeyRelease) {
    ButtonReleased(event->key());
  }
}

void ApplicationImpl::SceneTimer() {
  DrawFrame(ForceScreenUpdate{false});
  timer->start(0);
}

} // namespace detail

namespace {
detail::world::World CreateWorld(std::vector<std::string>&& models) {
  detail::world::WorldBuilder world_builder;
  detail::world::LocalObject local = detail::parser::Parse(models[0], models[1]);
  local.Normalize(1);
  detail::world::GlobalObject model_global(std::move(local), glm::vec3{0, 0, -10}, 1);
  world_builder.AddObject(std::move(model_global));
  // for (auto& model : models) {
  //   detail::world::LocalObject local = detail::parser::Parse(model, "");
  //   local.Normalize(1);
  //   detail::world::GlobalObject model_global(std::move(local), glm::vec3{0, 0, -10}, 1);
  //   world_builder.AddObject(std::move(model_global));
  // }
  return world_builder.Extract();
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
  impl.DrawFrame(ForceScreenUpdate{false});
}

void Application::Show() {
  impl.StartRenderer();
  impl.show();
}
