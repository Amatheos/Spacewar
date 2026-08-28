#pragma once

#include "engine/core/input.h"
#include "engine/core/math.h"

namespace se::render {

struct CameraSettings {
  float pan_speed = 160.0f;
  float zoom_rate = 1.25f;
  float min_zoom = 0.25f;
  float max_zoom = 8.0f;
  Key pan_left = Key::A;
  Key pan_right = Key::D;
  Key pan_up = Key::W;
  Key pan_down = Key::S;
  Key zoom_in = Key::E;
  Key zoom_out = Key::Q;
  Key reset = Key::R;
};

class Camera2D {
 public:
  explicit Camera2D(Bounds world, CameraSettings settings = {});

  void Update(float dt, const InputState& input);
  Vec2 center() const { return center_; }
  Vec2 HalfExtent() const { return base_extent_ / zoom_; }

 private:
  CameraSettings settings_;
  Vec2 base_extent_{0.0f, 0.0f};
  Vec2 center_{0.0f, 0.0f};
  float zoom_ = 1.0f;
};

}  // namespace se::render
