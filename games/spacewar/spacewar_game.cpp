#include "spacewar_game.h"

#include "engine/platform/window.h"

namespace spacewar {

namespace {
sim::GameInput MapGameInput(const se::InputState& in) {
  using se::Key;
  sim::GameInput g;

  sim::ShipInput& p1 = g.players[0];  // needle
  p1.rotate_left = in.IsDown(Key::A);
  p1.rotate_right = in.IsDown(Key::D);
  p1.thrust = in.IsDown(Key::W);
  p1.fire = in.IsDown(Key::LeftShift);
  p1.hyperspace = in.IsDown(Key::LeftCtrl);

  sim::ShipInput& p2 = g.players[1];  // wedge
  p2.rotate_left = in.IsDown(Key::Left);
  p2.rotate_right = in.IsDown(Key::Right);
  p2.thrust = in.IsDown(Key::Up);
  p2.fire = in.IsDown(Key::RightShift);
  p2.hyperspace = in.IsDown(Key::RightCtrl);

  return g;
}

app::MenuInput MapMenuInput(const se::InputState& in) {
  app::MenuInput m;
  using se::Key;

  m.up = in.IsDown(Key::W) || in.IsDown(Key::Up);
  m.down = in.IsDown(Key::S) || in.IsDown(Key::Down);
  m.left = in.IsDown(Key::A) || in.IsDown(Key::Left);
  m.right = in.IsDown(Key::D) || in.IsDown(Key::Right);
  m.select = in.IsDown(Key::Enter);
  m.back = in.IsDown(Key::Escape);
  m.debug = in.IsDown(Key::F1);
  return m;
}
}  // namespace

bool SpacewarGame::Init(se::Services& svs) {
  sim::SimSettings settings;
  if (!sim::LoadSimSettings(SPACEWAR_SETTINGS_PATH, settings)) return false;
  app::AppOptions options;
  app::LoadAppOptions(SPACEWAR_OPTIONS_PATH, options);

  app_.emplace(settings, options, svs.audio);
  app_->SetFullscreenHandler(
      [&window = svs.window](bool on) { window.SetFullscreen(on); });
  svs.window.SetFullscreen(options.fullscreen);
  return true;
}

void SpacewarGame::Update(float dt, const se::InputState& input) {
  app_->Update(dt, MapGameInput(input), MapMenuInput(input));
}

void SpacewarGame::BuildFrame(se::render::Frame& frame) {
  app_->BuildInto(frame);
}

}  // namespace spacewar

std::unique_ptr<se::Game> se::CreateGame() {
  return std::make_unique<spacewar::SpacewarGame>();
}
