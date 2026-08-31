#include "app/app.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

#include "app/format.h"

namespace spacewar::app {

using namespace se;

namespace {

constexpr Color kBackground{0.02f, 0.02f, 0.05f, 1.0f};
constexpr float kReloadInterval = 0.5f;  // hot-reload poll period, seconds
constexpr const char* kThemePath = SPACEWAR_DATA_DIR "main_menu.wav";
constexpr const char* kThrusterPath = SPACEWAR_DATA_DIR "thruster.wav";

constexpr Color kSel{1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color kDim{0.55f, 0.55f, 0.62f, 1.0f};
constexpr float kVolumeStep = 0.05f;
constexpr float kMatchStep = 15.0f;
constexpr float kMatchMin = 30.0f;
constexpr float kMatchMax = 600.0f;

constexpr float kPercentScale = 100.0f;

constexpr float kTitleY = 0.32f;
constexpr float kTitleH = 0.16f;
constexpr float kMenuFirstY = 0.56f;
constexpr float kRowStep = 0.10f;
constexpr float kSettingsTitleY = 0.24f;
constexpr float kSettingsTitleH = 0.10f;
constexpr float kSettingsFirstY = 0.50f;
constexpr float kGameOverY = 0.42f;
constexpr float kGameOverH = 0.12f;
constexpr float kHintFirstY = 0.56f;
constexpr float kHintStep = 0.07f;
constexpr float kHintH = 0.05f;

enum MenuRow { kNewGame, kSettingsEntry, kQuit, kMenuRowCount };
enum SettingsRow {
  kVolume,
  kMatchDuration,
  kFullscreen,
  kBack,
  kSettingsRowCount
};

enum class Clip { None, Fire, Explosion, Hyperspace, Beep, Go, kCount };

Clip ClipFor(sim::GameEvent::Kind kind) {
  using Kind = sim::GameEvent::Kind;
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

App::App(const sim::SimSettings& settings, const AppOptions& options,
         audio::Engine& audio)
    : settings_(settings),
      options_(options),
      font_(SPACEWAR_FONT_PATH),
      audio_(audio),
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

void App::Update(float dt, const sim::GameInput& game_in,
                 const MenuInput& menu_in) {
  reload_timer_ += dt;
  if (reload_timer_ >= kReloadInterval) {
    reload_timer_ = 0.0f;
    sim::LoadSimSettings(SPACEWAR_SETTINGS_PATH, settings_);
  }

  const bool was_in_game = in_game();
  HandleMenu(menu_in);
  if (quit_) return;

  const bool playing = in_game();
  if (playing && !was_in_game)
    audio_.StopPlayingLooped();
  else if (!playing && was_in_game)
    audio_.PlayLooped(kThemePath);

  if (playing && world_.game_phase() != sim::World::GamePhase::Finished) {
    world_.Update(dt, game_in);
    effects_.Update(dt);
    effects_.Consume(world_.events());
    PlayEvents();
  }

  bool thrusting = false;
  if (playing && world_.game_phase() == sim::World::GamePhase::Simulating)
    for (const sim::Ship& ship : world_.ships())
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
      constexpr int kCount = kMenuRowCount;
      if (up) menu_index_ = (menu_index_ + kCount - 1) % kCount;
      if (down) menu_index_ = (menu_index_ + 1) % kCount;
      if (select) {
        if (menu_index_ == kNewGame) {
          screen_ = Screen::Playing;
          world_ = sim::World(settings_);
        } else if (menu_index_ == kSettingsEntry) {
          screen_ = Screen::Settings;
          settings_index_ = kVolume;
        } else {
          quit_ = true;
        }
      }
      break;
    }
    case Screen::Settings: {
      constexpr int kCount = kSettingsRowCount;
      if (up) settings_index_ = (settings_index_ + kCount - 1) % kCount;
      if (down) settings_index_ = (settings_index_ + 1) % kCount;
      if (settings_index_ == kVolume && (left || right)) {
        options_.master_volume = std::clamp(
            options_.master_volume + (right ? kVolumeStep : -kVolumeStep), 0.0f,
            1.0f);
        audio_.SetVolume(options_.master_volume);
        SaveAppOptions(SPACEWAR_OPTIONS_PATH, options_);
      } else if (settings_index_ == kMatchDuration && (left || right)) {
        settings_.match.match_sec = std::clamp(
            settings_.match.match_sec + (right ? kMatchStep : -kMatchStep),
            kMatchMin, kMatchMax);
        sim::SaveSimSettings(SPACEWAR_SETTINGS_PATH, settings_);
      } else if (settings_index_ == kFullscreen && (left || right)) {
        options_.fullscreen = right;
        SaveAppOptions(SPACEWAR_OPTIONS_PATH, options_);
        if (fullscreen_handler_) fullscreen_handler_(options_.fullscreen);
      }
      if (back || (select && settings_index_ == kBack)) screen_ = Screen::Menu;
      break;
    }
    case Screen::Playing: {
      if (world_.game_phase() == sim::World::GamePhase::Finished) {
        if (select)
          world_ = sim::World(settings_);
        else if (back)
          screen_ = Screen::Menu;
      } else if (back) {
        screen_ = Screen::Menu;
      }
      break;
    }
  }

  game_over_ = (screen_ == Screen::Playing &&
                world_.game_phase() == sim::World::GamePhase::Finished);
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
  auto menu_y = [](int i) { return kMenuFirstY + kRowStep * i; };
  auto row_y = [](int i) { return kSettingsFirstY + kRowStep * i; };

  switch (screen_) {
    case Screen::Menu:
      frame.background = starscape_.get();
      line("SPACEWAR", kTitleY, kTitleH, kSel);
      line("NEW GAME", menu_y(kNewGame), kRowH, tint(menu_index_ == kNewGame));
      line("SETTINGS", menu_y(kSettingsEntry), kRowH,
           tint(menu_index_ == kSettingsEntry));
      line("QUIT", menu_y(kQuit), kRowH, tint(menu_index_ == kQuit));
      break;
    case Screen::Settings: {
      frame.background = starscape_.get();
      line("SETTINGS", kSettingsTitleY, kSettingsTitleH, kSel);
      int pct = static_cast<int>(options_.master_volume * kPercentScale + 0.5f);
      row("VOLUME", std::to_string(pct) + "%", row_y(kVolume),
          settings_index_ == kVolume);
      row("MATCH DURATION", FormatClock(settings_.match.match_sec),
          row_y(kMatchDuration), settings_index_ == kMatchDuration);
      row("FULLSCREEN", options_.fullscreen ? "On" : "Off", row_y(kFullscreen),
          settings_index_ == kFullscreen);
      line("BACK", row_y(kBack), kRowH, tint(settings_index_ == kBack));
      break;
    }
    case Screen::Playing:
      if (game_over_) {
        line("GAME OVER", kGameOverY, kGameOverH, kSel);
        line("PRESS ENTER TO RESTART", kHintFirstY, kHintH, kSel);
        line("ESC RETURN TO MENU", kHintFirstY + kHintStep, kHintH, kSel);
      }
      break;
  }
}

void App::BuildInto(render::Frame& frame) const {
  frame.tick = world_.tick();
  frame.clear_color = kBackground;
  frame.world_half = {sim::World::kHalfWidth, sim::World::kHalfHeight};
  if (in_game()) {
    game_view_.BuildInto(frame, world_, show_colliders_);
    effects_.BuildInto(frame);
    hud_.BuildInto(frame, world_);
  }
  DrawMenu(frame);
}

}  // namespace spacewar::app
