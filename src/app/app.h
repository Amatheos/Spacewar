#pragma once

#include <functional>
#include <memory>

#include "app/effects.h"
#include "app/game_view.h"
#include "app/hud.h"
#include "app/options.h"
#include "audio/engine.h"
#include "core/input.h"
#include "game/settings.h"
#include "game/world.h"
#include "render/font.h"
#include "render/frame.h"
#include "render/texture.h"

namespace spacewar::app {

// The application: owns the resident World, the views, audio, and the menu
// state machine. main drives it via Update/BuildInto and owns only the window
// and renderer.
class App {
 public:
  App(const game::SimSettings& settings, const AppOptions& options);

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void Update(float dt, const GameInput& game_in, const MenuInput& menu_in);
  void BuildInto(render::Frame& frame) const;
  bool should_quit() const { return quit_; }

  void SetFullscreenHandler(std::function<void(bool)> cb);

 private:
  enum class Screen { Menu, Settings, Playing };

  bool in_game() const { return screen_ == Screen::Playing; }
  void HandleMenu(const MenuInput& in);
  void DrawMenu(render::Frame& frame) const;
  void PlayEvents();

  game::SimSettings settings_;
  AppOptions options_;
  render::Font font_;
  audio::Engine audio_;
  game::World world_;
  GameView game_view_;
  Effects effects_;
  Hud hud_;
  std::unique_ptr<render::Texture> starscape_;

  Screen screen_ = Screen::Menu;
  int menu_index_ = 0;
  int settings_index_ = 0;
  bool game_over_ = false;
  bool quit_ = false;
  bool show_colliders_ = false;
  MenuInput prev_menu_{};
  float accumulator_ = 0.0f;
  float reload_timer_ = 0.0f;
  std::function<void(bool)> fullscreen_handler_;
};

}  // namespace spacewar::app
