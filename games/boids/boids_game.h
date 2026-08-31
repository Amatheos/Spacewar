#pragma once

#include "app/flock_view.h"
#include "engine/core/math.h"
#include "engine/game.h"
#include "engine/render/camera.h"
#include "sim/flock.h"

namespace boids {
inline constexpr float kUpdateHz = 120.0f;
inline constexpr se::Bounds kWorldBounds{320.0f, 180.0f};

class BoidsGame : public se::Game {
 public:
  BoidsGame() : Game(kUpdateHz), flock_(sim::FlockSettings{}, kWorldBounds) {}

  bool Init(se::Services& svs) override;
  void Update(float dt, const se::InputState& input) override;
  void BuildFrame(se::render::Frame& frame) override;
  bool WantsExit() const override { return quit_; }

 private:
  sim::Flock flock_;
  app::FlockView view_;
  se::render::Camera2D camera_{kWorldBounds};
  bool quit_ = false;
};

}  // namespace boids
