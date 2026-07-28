#pragma once

#include <functional>
#include <memory>

#include "app/effects.h"
#include "app/game_view.h"
#include "app/hud.h"
#include "app/options.h"
#include "engine/audio/engine.h"
#include "engine/render/font.h"
#include "engine/render/frame.h"
#include "engine/render/texture.h"
#include "sim/settings.h"
#include "sim/world.h"

namespace spacewar::app {

struct MenuInput {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool select = false;
  bool back = false;
  bool debug = false;
};

// The application: owns the resident World, the views, audio, and the menu
// state machine. main drives it via Update/BuildInto and owns only the window
// and renderer.
class App {
 public:
  App(const sim::SimSettings& settings, const AppOptions& options,
      se::audio::Engine& audio);

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void Update(float dt, const sim::GameInput& game_in,
              const MenuInput& menu_in);
  void BuildInto(se::render::Frame& frame) const;
  bool should_quit() const { return quit_; }

  void SetFullscreenHandler(std::function<void(bool)> cb);

 private:
  enum class Screen { Menu, Settings, Playing };

  bool in_game() const { return screen_ == Screen::Playing; }
  void HandleMenu(const MenuInput& in);
  void DrawMenu(se::render::Frame& frame) const;
  void PlayEvents();

  sim::SimSettings settings_;
  AppOptions options_;
  se::render::Font font_;
  se::audio::Engine& audio_;
  sim::World world_;
  GameView game_view_;
  Effects effects_;
  Hud hud_;
  std::unique_ptr<se::render::Texture> starscape_;

  Screen screen_ = Screen::Menu;
  int menu_index_ = 0;
  int settings_index_ = 0;
  bool game_over_ = false;
  bool quit_ = false;
  bool show_colliders_ = false;
  MenuInput prev_menu_{};
  float reload_timer_ = 0.0f;
  std::function<void(bool)> fullscreen_handler_;
};

}  // namespace spacewar::app
