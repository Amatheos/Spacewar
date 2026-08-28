#include "engine/render/camera.h"

#include <algorithm>

namespace se::render {

Camera2D::Camera2D(Bounds world, CameraSettings settings)
    : settings_(settings), base_extent_{world.half_w, world.half_h} {}

void Camera2D::Update(float dt, const InputState& input) {
  if (input.WasPressed(settings_.zoom_in)) {
    zoom_ *= settings_.zoom_rate;
  }

  if (input.WasPressed(settings_.zoom_out)) {
    zoom_ /= settings_.zoom_rate;
  }

  if (input.WasPressed(settings_.reset)) {
    zoom_ = 1.0f;
    center_ = {0, 0};
  }

  const float step = settings_.pan_speed * dt / zoom_;
  Vec2 dir{0.0f, 0.0f};

  if (input.IsDown(settings_.pan_right)) dir.x += 1.0f;
  if (input.IsDown(settings_.pan_up)) dir.y += 1.0f;
  if (input.IsDown(settings_.pan_left)) dir.x -= 1.0f;
  if (input.IsDown(settings_.pan_down)) dir.y -= 1.0f;

  center_ += dir.Normalized() * step;

  zoom_ = std::clamp(zoom_, settings_.min_zoom, settings_.max_zoom);
  center_.x = std::clamp(center_.x, -base_extent_.x, base_extent_.x);
  center_.y = std::clamp(center_.y, -base_extent_.y, base_extent_.y);
}

}  // namespace se::render