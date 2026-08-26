#include "boids_game.h"

#include <memory>

#include "engine/core/input.h"
#include "engine/render/frame.h"

namespace boids {

namespace {
constexpr se::Color kBackground{0.02f, 0.03f, 0.06f, 1.0f};
}

bool BoidsGame::Init(se::Services&) { return true; }

void BoidsGame::Update(float dt, const se::InputState& input) {
  if (input.IsDown(se::Key::Escape)) quit_ = true;
  flock_.Update(dt);
}

void BoidsGame::BuildFrame(se::render::Frame& frame) {
  frame.clear_color = kBackground;
  frame.world_half = {kWorldBounds.half_w, kWorldBounds.half_h};
  view_.BuildInto(frame, flock_);
}

}  // namespace boids

std::unique_ptr<se::Game> se::CreateGame() {
  return std::make_unique<boids::BoidsGame>();
}
