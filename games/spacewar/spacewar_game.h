#pragma once
#include <optional>

#include "app/app.h"
#include "engine/game.h"

namespace spacewar {
inline constexpr float kUpdateHz = 120.0f;

class SpacewarGame : public se::Game {
 public:
  SpacewarGame() : Game(kUpdateHz) {}

  bool Init(se::Services& svs) override;
  void Update(float dt, const se::InputState& input) override;
  void BuildFrame(se::render::Frame& frame) override;
  bool WantsExit() const override { return app_->should_quit(); }

 private:
  std::optional<app::App> app_;
};

}  // namespace spacewar
