
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
    Height height, Width width, Height shadow_buffer_height, Width shadow_buffer_width
)
    : QWidget(nullptr),
      world(std::move(world_)),
      direction_light(std::move(light)),
      camera(std::move(camera_)),
      screen(height, width),
      renderer(
          threads_count, screen.GetHeight(), screen.GetWidth(), &camera, &world, &direction_light,
          shadow_buffer_height, shadow_buffer_width
      ),
      layout_(new QVBoxLayout(this)),
      timer_(new QTimer()),
      frame_drawing_timer_(new QElapsedTimer()) {
  // world.GenerateBoundingBox(direction_light);

  setFocusPolicy(Qt::StrongFocus);

  layout_->addWidget(&screen);

  connect(timer_, SIGNAL(timeout()), this, SLOT(SceneTimer()));
  printf("Total triangles: %d\n", world.GetTrianglesCapacity());
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
  } else if (button == Qt::Key_Q) {
    c = 'q';
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

inline bool IsMaterialChangePressed(char c) {
  return c == 'q';
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
  } else if (IsMaterialChangePressed(c)) {
    renderer.ChangeMaterialStatus();
    // printf("Pressed\n");
    require_screen_update = true;
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
  if (camera.IsMoving() || camera.IsRotating() || camera.IsReset() || require_screen_update ||
      flag()) {
    if (camera.IsReset()) {
      camera.Reset();
    }

    if (require_screen_update) {
      require_screen_update = false;
    }
    camera.UpdateCameraMatirx();
    direction_light.UpdateDirection(camera);

    const Frame& new_frame = renderer.MakeFrame();
    screen.DrawFrameWithFps(new_frame, one_second / float(frame_drawing_timer_->nsecsElapsed()));
    renderer.ChangeShadowMapUpdateStatus(renderer::ShadowMapUpdateRequired::Not_Required);
  }
}

void ApplicationImpl::StartRenderer() {
  renderer.ChangeShadowMapUpdateStatus(renderer::ShadowMapUpdateRequired::Required);
  DrawFrame(ForceScreenUpdate{true});
  timer_->start(0);
}

void ApplicationImpl::SetRequireUpdate() {
  require_screen_update = true;
}

void ApplicationImpl::DropRequireUpdate() {
  require_screen_update = false;
}

ApplicationImpl::~ApplicationImpl() {
  delete layout_;
  delete timer_;
}

void ApplicationImpl::keyPressEvent(QKeyEvent* event) {
  // printf("Happend\n");
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
  // renderer.ChangeShadowMapUpdateStatus(renderer::ShadowMapUpdateRequired::Not_Required);
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
        models[i].path_to_obj(), models[i].path_to_texture(), models[i].path_to_material(),
        models[i].bfc_status
    );

    local.Normalize(1);
    float z_shift = local.FindMaxZ();
    detail::world::GlobalObject model_global(
        std::move(local), models[i].shift() - V3{0, 0, z_shift}, models[i].transformation()
    );
    world_builder.AddObject(std::move(model_global));
  }
  return world_builder.Extract();
}

} // namespace

Application::Application(
    ThreadsCount threads_count, std::vector<Model>&& models, DirectionalLightSource&& light,
    Height height, Width width, HorizontalFOV hf, NearPlaneDistance npd, RenderDistance rd,
    Height shadow_buffer_height, Width shadow_buffer_width
)
    : impl(
          std::make_unique<ApplicationImpl>(
              threads_count(), CreateWorld(std::move(models)), std::move(light),
              Camera(
                  hf, AspectRatio{static_cast<float>(height()) / static_cast<float>(width())}, npd,
                  rd
              ),

              height, width, shadow_buffer_height, shadow_buffer_width
          )
      ) {}

void Application::Run() {
  impl->StartRenderer();
  impl->show();
}
