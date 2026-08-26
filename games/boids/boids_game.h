#pragma once

#include "app/flock_view.h"
#include "engine/core/math.h"
#include "engine/game.h"
#include "sim/flock.h"

namespace boids {
inline constexpr float kUpdateHz = 120.0f;
inline constexpr se::Bounds kWorldBounds{100.0f, 56.25f};

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
  bool quit_ = false;
};

}  // namespace boids
