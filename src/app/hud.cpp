#include "app/hud.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>

#include "app/format.h"
#include "app/player_style.h"
#include "core/math.h"
#include "game/player.h"
#include "game/settings.h"

namespace spacewar::app {

namespace {

// All fractions of the shorter window axis (the overlay's isotropic unit).
constexpr float kPad = 0.02f;    // edge margin
constexpr float kTextH = 0.05f;  // readout text height
constexpr float kGap = 0.012f;   // score -> bar gap
constexpr float kBarW = 0.18f;   // cooldown bar
constexpr float kBarH = 0.022f;

}  // namespace

Hud::Hud(const char* font_path) : font_(font_path) {}

void Hud::BuildInto(render::Frame& frame, const game::World& world) const {
  const Vec2 extent = frame.overlay_extent;
  const float cd_max = world.settings().hyperspace.cooldown_sec;
  const std::array<int, game::kPlayerCount>& scores = world.player_score();

  // Per-player column: score readout + hyperspace cooldown bar. Needle anchors
  // to the left edge, wedge mirrors to the right.
  for (std::size_t i = 0; i < game::kPlayerCount; ++i) {
    std::string score = std::to_string(scores[i]);
    float score_w = font_.MeasureWidth(score, kTextH);
    float col_x = (i == 0) ? kPad : extent.x - kPad - kBarW;
    float score_x = (i == 0) ? kPad : extent.x - kPad - score_w;

    font_.AppendText(frame, score, {score_x, kPad + kTextH}, kTextH,
                     kPlayerStyles[i].hull);

    float remaining = world.hyperspace_cooldown(static_cast<game::Player>(i));
    float ready = cd_max > 0.0f
                      ? 1.0f - std::clamp(remaining / cd_max, 0.0f, 1.0f)
                      : 1.0f;

    render::DrawCommand bg;
    bg.kind = render::DrawCommand::Kind::Rect;
    bg.pos = {col_x, kPad + kTextH + kGap};
    bg.size = {kBarW, kBarH};
    bg.color = {0.15f, 0.15f, 0.20f, 0.6f};
    frame.overlay.push_back(bg);

    render::DrawCommand fill = bg;
    fill.size = {kBarW * ready, kBarH};
    fill.color = kPlayerStyles[i].hull;
    frame.overlay.push_back(fill);
  }

  // Match clock, centered along the top.
  std::string clock = FormatClock(world.match_timer());
  float clock_w = font_.MeasureWidth(clock, kTextH);
  font_.AppendText(frame, clock,
                   {extent.x * 0.5f - clock_w * 0.5f, kPad + kTextH}, kTextH,
                   {0.85f, 0.85f, 0.90f, 1.0f});

  // Big centered banner: the between-round countdown, then GAME OVER.
  auto center = [&](std::string_view text, float h) {
    float w = font_.MeasureWidth(text, h);
    font_.AppendText(frame, text,
                     {extent.x * 0.5f - w * 0.5f, extent.y * 0.5f + h * 0.35f},
                     h, {1.0f, 1.0f, 1.0f, 1.0f});
  };
  if (world.game_phase() == game::World::GamePhase::Countdown) {
    int n = static_cast<int>(std::ceil(world.respawn_timer()));
    if (n > 0) center(std::to_string(n), 0.28f);
  }
}

}  // namespace spacewar::app
