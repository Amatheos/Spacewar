#include "boids_game.h"

#include <memory>

#include "engine/core/input.h"
#include "engine/render/frame.h"

namespace boids {

namespace {
constexpr se::Color kBackground{0.02f, 0.03f, 0.06f, 1.0f};
constexpr se::Color kWorldBorder{1.0f, 0.0f, 0.0f, 1.0f};
constexpr float kWorldBorderThickness = 0.5f;

}  // namespace

bool BoidsGame::Init(se::Services&) { return true; }

void BoidsGame::Update(float dt, const se::InputState& input) {
  if (input.IsDown(se::Key::Escape)) quit_ = true;
  camera_.Update(dt, input);
  flock_.Update(dt);
}

void BoidsGame::BuildFrame(se::render::Frame& frame) {
  frame.clear_color = kBackground;
  frame.world_half = camera_.HalfExtent();
  frame.view_center = camera_.center();
  frame.commands.push_back(
      {.kind = se::render::DrawCommand::Kind::Rect,
       .pos = {-kWorldBounds.half_w, -kWorldBounds.half_h},
       .color = kWorldBorder,
       .thickness = kWorldBorderThickness,
       .size = {kWorldBounds.half_w * 2.0f, kWorldBounds.half_h * 2.0f}});
  view_.BuildInto(frame, flock_);
}

}  // namespace boids

std::unique_ptr<se::Game> se::CreateGame() {
  return std::make_unique<boids::BoidsGame>();
}
