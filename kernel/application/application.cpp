
#include "application/application.h"

#include "QDebug"
#include "camera/camera.h"
#include "concurrency/concurrency.h"
#include "light/light.h"
#include "parser/parser.h"
#include "renderer/renderer.h"
#include "screen/screen.h"
#include "types/types.h"
#include "world/world.h"

#include <cassert>
#include <cstdio>
#include <memory>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qlogging.h>
#include <unordered_map>

namespace detail {

ApplicationImpl::ApplicationImpl(
    int32_t threads_count, World&& world_, DirectionalLightSource&& light, Camera&& camera_,
    Height height, Width width
)
    : QWidget(nullptr),
      world(std::move(world_)),
      direction_light(std::move(light)),
      camera(std::move(camera_)),
      screen(height, width),
      renderer(
          threads_count, screen.GetHeight(), screen.GetWidth(), &camera, &world, &direction_light
      ),
      layout_(new QVBoxLayout(this)),
      timer_(new QTimer()),
      frame_drawing_timer_(new QElapsedTimer()) {
  setFocusPolicy(Qt::StrongFocus);

  layout_->addWidget(&screen);

  connect(timer_, SIGNAL(timeout()), this, SLOT(SceneTimer()));
  /*
  void Screen::Connect(QVBoxLayout* layout) {
  layout->addWidget(screen.get());
}
  */
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

using Rotating = camera::Rotating;
using Moving = camera::Moving;
static std::unordered_map<char, Rotating> rotating_mapping = {
    {'i', Rotating::Up}, {'k', Rotating::Down}, {'j', Rotating::Left}, {'l', Rotating::Right}
};

static std::unordered_map<char, Moving> moving_mapping = {
    {'w', Moving::Toward}, {'s', Moving::Backward}, {'a', Moving::Left}, {'d', Moving::Right}
};

} // namespace

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

static const float one_second = 1000000000.0f;
void ApplicationImpl::DrawFrame(ForceScreenUpdate flag) {
  frame_drawing_timer_->start();
  if (camera.IsMoving() || camera.IsRotating() || camera.IsReset() || flag()) {
    if (camera.IsReset()) {
      camera.ResetComplete();
    }

    camera.UpdateCameraMatirx();
    direction_light.UpdateDirection(camera);

    const Frame& new_frame = renderer.MakeFrame();
    screen.DrawFrameWithFps(new_frame, one_second / float(frame_drawing_timer_->nsecsElapsed()));
  }
}

// void ApplicationImpl::ConnectScreen(QVBoxLayout* layout) {
//   screen.Connect(layout);
// }

void ApplicationImpl::StartRenderer() {
  DrawFrame(ForceScreenUpdate{true});
  timer_->start(0);
}

ApplicationImpl::~ApplicationImpl() {
  delete layout_;
  delete timer_;
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
  timer_->start(0);
}

} // namespace detail

namespace {
using World = detail::world::World;

World CreateWorld(std::vector<Model>&& models) {
  detail::world::WorldBuilder world_builder;

  for (int32_t i = 0; i < models.size(); ++i) {
    detail::world::LocalObject local = detail::parser::Parse(
        models[i].path_to_obj(), models[i].path_to_texture(), models[i].bfc_status
    );
    local.Normalize(1);
    detail::world::GlobalObject model_global(std::move(local), glm::vec3{0, 0, -10 * i}, 1);
    world_builder.AddObject(std::move(model_global));
  }
  return world_builder.Extract();
}

} // namespace

Application::Application(
    ThreadsCount threads_count, std::vector<Model>&& models, DirectionalLightSource&& light,
    Height height, Width width, HorizontalFOV hf, NearPlaneDistance npd, RenderDistance rd
)
    : impl(
          std::make_unique<ApplicationImpl>(
              threads_count(), CreateWorld(std::move(models)), std::move(light),
              Camera(
                  hf, AspectRatio{static_cast<float>(height()) / static_cast<float>(width())}, npd,
                  rd
              ),

              height, width
          )
      ) {}

void Application::Run() {
  impl->StartRenderer();
  impl->show();
}
