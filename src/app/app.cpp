#include "app/app.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "app/format.h"

namespace spacewar::app {

namespace {

constexpr float kFixedStep = 1.0f / 120.0f;
constexpr float kMaxFrame = 0.25f;
constexpr float kReloadInterval = 0.5f;  // hot-reload poll period, seconds
constexpr const char* kThemePath = SPACEWAR_DATA_DIR "main_menu.wav";
constexpr const char* kThrusterPath = SPACEWAR_DATA_DIR "thruster.wav";

constexpr Color kSel{1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color kDim{0.55f, 0.55f, 0.62f, 1.0f};
constexpr float kVolumeStep = 0.05f;
constexpr float kMatchStep = 15.0f;
constexpr float kMatchMin = 30.0f;
constexpr float kMatchMax = 600.0f;

enum class Clip { None, Fire, Explosion, Hyperspace, Beep, Go, kCount };

Clip ClipFor(game::GameEvent::Kind kind) {
  using Kind = game::GameEvent::Kind;
  switch (kind) {
    case Kind::TorpedoFired:
      return Clip::Fire;
    case Kind::ShipExploded:
      return Clip::Explosion;
    case Kind::HyperspaceDepart:
      return Clip::Hyperspace;
    case Kind::CountdownTick:
      return Clip::Beep;
    case Kind::CountdownGo:
      return Clip::Go;
    case Kind::HyperspaceArrive:
      return Clip::None;
    case Kind::TorpedoDetonated:
      return Clip::None;
    case Kind::TorpedoExpired:
      return Clip::None;
  }
  return Clip::None;
}

const char* PathFor(Clip clip) {
  switch (clip) {
    case Clip::Fire:
      return SPACEWAR_DATA_DIR "fire.wav";
    case Clip::Explosion:
      return SPACEWAR_DATA_DIR "explosion.wav";
    case Clip::Hyperspace:
      return SPACEWAR_DATA_DIR "hyperspace.wav";
    case Clip::Beep:
      return SPACEWAR_DATA_DIR "beep.wav";
    case Clip::Go:
      return SPACEWAR_DATA_DIR "go.wav";
    default:
      return nullptr;
  }
}

}  // namespace

App::App(const game::SimSettings& settings, const AppOptions& options)
    : settings_(settings),
      options_(options),
      font_(SPACEWAR_FONT_PATH),
      world_(settings_),
      effects_(settings_.rng_seed),
      hud_(SPACEWAR_FONT_PATH),
      starscape_(render::LoadTextureFile(SPACEWAR_DATA_DIR "starscape.png")) {
  audio_.SetVolume(options_.master_volume);
  audio_.PlayLooped(kThemePath);
}

void App::SetFullscreenHandler(std::function<void(bool)> cb) {
  fullscreen_handler_ = std::move(cb);
}

void App::Update(float dt, const GameInput& game_in, const MenuInput& menu_in) {
  if (dt > kMaxFrame) dt = kMaxFrame;

  reload_timer_ += dt;
  if (reload_timer_ >= kReloadInterval) {
    reload_timer_ = 0.0f;
    game::LoadSimSettings(SPACEWAR_SETTINGS_PATH, settings_);
  }

  const bool was_in_game = in_game();
  HandleMenu(menu_in);
  if (quit_) return;

  const bool playing = in_game();
  if (playing && !was_in_game)
    audio_.StopPlayingLooped();
  else if (!playing && was_in_game)
    audio_.PlayLooped(kThemePath);

  if (playing && world_.game_phase() != game::World::GamePhase::Finished) {
    accumulator_ += dt;
    while (accumulator_ >= kFixedStep) {
      world_.Update(kFixedStep, game_in);
      effects_.Update(kFixedStep);
      effects_.Consume(world_.events());
      PlayEvents();
      accumulator_ -= kFixedStep;
    }
  } else {
    accumulator_ = 0.0f;
  }

  bool thrusting = false;
  if (playing && world_.game_phase() == game::World::GamePhase::Simulating)
    for (const game::Ship& ship : world_.ships())
      if (ship.thrusting()) {
        thrusting = true;
        break;
      }
  audio_.SetThruster(thrusting, kThrusterPath);
}

void App::HandleMenu(const MenuInput& in) {
  auto edge = [](bool now, bool before) { return now && !before; };
  const bool up = edge(in.up, prev_menu_.up);
  const bool down = edge(in.down, prev_menu_.down);
  const bool left = edge(in.left, prev_menu_.left);
  const bool right = edge(in.right, prev_menu_.right);
  const bool select = edge(in.select, prev_menu_.select);
  const bool back = edge(in.back, prev_menu_.back);
  const bool debug = edge(in.debug, prev_menu_.debug);
  prev_menu_ = in;

  if (debug) show_colliders_ = !show_colliders_;

  switch (screen_) {
    case Screen::Menu: {
      constexpr int kCount = 3;
      if (up) menu_index_ = (menu_index_ + kCount - 1) % kCount;
      if (down) menu_index_ = (menu_index_ + 1) % kCount;
      if (select) {
        if (menu_index_ == 0) {
          screen_ = Screen::Playing;
          world_ = game::World(settings_);
        } else if (menu_index_ == 1) {
          screen_ = Screen::Settings;
          settings_index_ = 0;
        } else {
          quit_ = true;
        }
      }
      break;
    }
    case Screen::Settings: {
      constexpr int kCount = 4;
      if (up) settings_index_ = (settings_index_ + kCount - 1) % kCount;
      if (down) settings_index_ = (settings_index_ + 1) % kCount;
      if (settings_index_ == 0 && (left || right)) {
        options_.master_volume = std::clamp(
            options_.master_volume + (right ? kVolumeStep : -kVolumeStep), 0.0f,
            1.0f);
        audio_.SetVolume(options_.master_volume);
        SaveAppOptions(SPACEWAR_OPTIONS_PATH, options_);
      } else if (settings_index_ == 1 && (left || right)) {
        settings_.match.match_sec = std::clamp(
            settings_.match.match_sec + (right ? kMatchStep : -kMatchStep),
            kMatchMin, kMatchMax);
        game::SaveSimSettings(SPACEWAR_SETTINGS_PATH, settings_);
      } else if (settings_index_ == 2 && (left || right)) {
        options_.fullscreen = right;
        SaveAppOptions(SPACEWAR_OPTIONS_PATH, options_);
        if (fullscreen_handler_) fullscreen_handler_(options_.fullscreen);
      }
      if (back || (select && settings_index_ == 3)) screen_ = Screen::Menu;
      break;
    }
    case Screen::Playing: {
      if (world_.game_phase() == game::World::GamePhase::Finished) {
        if (select)
          world_ = game::World(settings_);
        else if (back)
          screen_ = Screen::Menu;
      } else if (back) {
        screen_ = Screen::Menu;
      }
      break;
    }
  }

  game_over_ = (screen_ == Screen::Playing &&
                world_.game_phase() == game::World::GamePhase::Finished);
}

void App::PlayEvents() {
  bool played[static_cast<int>(Clip::kCount)] = {};
  for (const auto& e : world_.events()) {
    Clip clip = ClipFor(e.kind);
    if (clip == Clip::None || played[static_cast<int>(clip)]) continue;
    played[static_cast<int>(clip)] = true;
    audio_.PlayOnce(PathFor(clip));
  }
}

void App::DrawMenu(render::Frame& frame) const {
  const float cx = frame.overlay_extent.x * 0.5f;
  auto line = [&](std::string_view s, float y, float h, const Color& c) {
    float w = font_.MeasureWidth(s, h);
    font_.AppendText(frame, s, {cx - w * 0.5f, y}, h, c);
  };
  auto tint = [](bool sel) { return sel ? kSel : kDim; };
  constexpr float kRowH = 0.06f;
  constexpr float kRowHalf = 0.55f;
  auto row = [&](std::string_view label, std::string_view value, float y,
                 bool sel) {
    const Color c = tint(sel);
    font_.AppendText(frame, label, {cx - kRowHalf, y}, kRowH, c);
    float vw = font_.MeasureWidth(value, kRowH);
    font_.AppendText(frame, value, {cx + kRowHalf - vw, y}, kRowH, c);
  };

  switch (screen_) {
    case Screen::Menu:
      frame.background = starscape_.get();
      line("SPACEWAR", 0.32f, 0.16f, kSel);
      line("NEW GAME", 0.56f, 0.06f, tint(menu_index_ == 0));
      line("SETTINGS", 0.66f, 0.06f, tint(menu_index_ == 1));
      line("QUIT", 0.76f, 0.06f, tint(menu_index_ == 2));
      break;
    case Screen::Settings: {
      frame.background = starscape_.get();
      line("SETTINGS", 0.24f, 0.10f, kSel);
      int pct = static_cast<int>(options_.master_volume * 100.0f + 0.5f);
      row("VOLUME", std::to_string(pct) + "%", 0.50f, settings_index_ == 0);
      row("MATCH DURATION", FormatClock(settings_.match.match_sec), 0.60f,
          settings_index_ == 1);
      row("FULLSCREEN", options_.fullscreen ? "On" : "Off", 0.70f,
          settings_index_ == 2);
      line("BACK", 0.80f, 0.06f, tint(settings_index_ == 3));
      break;
    }
    case Screen::Playing:
      if (game_over_) {
        line("GAME OVER", 0.42f, 0.12f, kSel);
        line("PRESS ENTER TO RESTART", 0.56f, 0.05f, kSel);
        line("ESC RETURN TO MENU", 0.63f, 0.05f, kSel);
      }
      break;
  }
}

void App::BuildInto(render::Frame& frame) const {
  frame.tick = world_.tick();
  if (in_game()) {
    game_view_.BuildInto(frame, world_, show_colliders_);
    effects_.BuildInto(frame);
    hud_.BuildInto(frame, world_);
  }
  DrawMenu(frame);
}

}  // namespace spacewar::app
