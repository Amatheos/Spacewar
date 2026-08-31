#include "app/game_view.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "app/player_style.h"
#include "engine/core/math.h"
#include "sim/player.h"
#include "sim/settings.h"

namespace spacewar::app {

using namespace se;

namespace {

// Draw scale for both ship silhouettes (presentation, not physics).
constexpr float kShipScale = 3.5f;

// Hyperspace ring: a violet outline that implodes onto the jumping ship, hits
// zero at the midpoint teleport, then explodes back out. Sized entirely off the
// jump timer, so it carries no state of its own.
constexpr float kRingMaxRadius = 9.0f;
constexpr float kRingThickness = 1.2f;  // band width, world units
constexpr Color kRingColor{0.70f, 0.55f, 1.0f, 1.0f};  // violet, additive glow

constexpr float kTorpedoRadius = 0.8f;

// Star glow: concentric discs, faint to bright, additively blended into a
// glowing core (radius multiplier, color).
struct StarLayer {
  float scale;
  Color color;
};
constexpr std::array<StarLayer, 3> kStarLayers{{
    {2.4f, Color{1.0f, 0.65f, 0.30f, 0.15f}},  // halo
    {1.5f, Color{1.0f, 0.80f, 0.40f, 0.35f}},  // mid
    {1.0f, Color{1.0f, 0.93f, 0.70f, 0.95f}},  // core
}};

constexpr Color kColliderColor{0.0f, 1.0f, 0.35f, 0.9f};  // debug hitbox green
constexpr float kColliderThickness = 0.25f;

constexpr float kFlameCoreScale = 0.6f;
constexpr float kFlameOuterAlpha = 0.40f;
constexpr float kFlameCoreAlpha = 0.90f;

constexpr std::uint32_t kHashMultiplier = 2654435761u;
constexpr int kHashShift = 15;
constexpr std::uint32_t kHashMask = 0xffffu;
constexpr float kHashMaskScale = 65535.0f;

constexpr float kFlickerBase = 0.75f;
constexpr float kFlickerAmplitude = 0.35f;

float Flicker(std::uint64_t tick) {
  std::uint32_t h = static_cast<std::uint32_t>(tick) * kHashMultiplier;
  h ^= h >> kHashShift;
  return (h & kHashMask) / kHashMaskScale;
}

}  // namespace

void GameView::BuildInto(render::Frame& frame, const sim::World& world,
                         bool show_colliders) const {
  const std::array<sim::Ship, sim::kPlayerCount>& ships = world.ships();
  for (std::size_t i = 0; i < ships.size(); ++i) {
    if (!ships[i].alive()) continue;  // exploded: hull vanishes until respawn

    float timer = world.hyperspace_timer(static_cast<sim::Player>(i));

    if (ships[i].thrusting() && timer <= 0.0f) {
      const float flick =
          kFlickerBase + kFlickerAmplitude * Flicker(world.tick());
      const Color ex = kPlayerStyles[i].exhaust;
      const Vec2 port =
          ships[i].pos() + Vec2::FromAngle(ships[i].angle()) *
                               (kPlayerStyles[i].engine * kShipScale);
      render::DrawCommand outer;
      outer.kind = render::DrawCommand::Kind::Polygon;
      outer.pos = port;
      outer.angle = ships[i].angle();
      outer.scale = kShipScale * flick;
      outer.shape = kPlayerStyles[i].flame;
      outer.color = {ex.r, ex.g, ex.b, kFlameOuterAlpha};
      frame.commands.push_back(outer);

      render::DrawCommand core = outer;
      core.scale = kShipScale * flick * kFlameCoreScale;
      core.color = {(ex.r + 1.0f) * 0.5f, (ex.g + 1.0f) * 0.5f,
                    (ex.b + 1.0f) * 0.5f, kFlameCoreAlpha};
      frame.commands.push_back(core);
    }

    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Polygon;
    cmd.pos = ships[i].pos();
    cmd.angle = ships[i].angle();
    cmd.scale = kShipScale;
    cmd.color = kPlayerStyles[i].hull;
    cmd.shape = kPlayerStyles[i].shape;
    frame.commands.push_back(cmd);

    // Hyperspace ring. Sits under the `alive()` continue above, so a jump that
    // malfunctions at the midpoint (ship dies) collapses the ring to nothing
    // rather than leaving it hanging. |timer - mid| / mid is 1 at both ends of
    // the jump and 0 at the midpoint, so the outline implodes onto ships[i]
    // then explodes back out from wherever it rematerialized.
    if (timer > 0.0f) {
      const float mid = world.settings().hyperspace.anim_duration_sec * 0.5f;
      render::DrawCommand ring;
      ring.kind = render::DrawCommand::Kind::Ring;
      ring.pos = ships[i].pos();
      ring.radius = kRingMaxRadius * std::fabs(timer - mid) / mid;
      ring.thickness = kRingThickness;
      ring.color = kRingColor;
      frame.commands.push_back(ring);
    }
  }

  const sim::Star& star = world.star();
  for (const StarLayer& layer : kStarLayers) {
    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Circle;
    cmd.pos = star.pos();
    cmd.radius = world.settings().star.radius * layer.scale;
    cmd.color = layer.color;
    frame.commands.push_back(cmd);
  }

  for (const sim::Torpedo& torpedo : world.torpedoes()) {
    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Circle;
    cmd.pos = torpedo.pos();
    cmd.radius = kTorpedoRadius;
    cmd.color = kPlayerStyles[idx(torpedo.owner())].torpedo;
    frame.commands.push_back(cmd);
  }

  if (show_colliders) {
    const sim::SimSettings& s = world.settings();
    auto collider = [&](Vec2 pos, float radius) {
      render::DrawCommand cmd;
      cmd.kind = render::DrawCommand::Kind::Ring;
      cmd.pos = pos;
      cmd.radius = radius;
      cmd.thickness = kColliderThickness;
      cmd.color = kColliderColor;
      frame.commands.push_back(cmd);
    };
    for (const sim::Ship& ship : ships)
      if (ship.alive()) collider(ship.pos(), s.ship.collision_radius);
    for (const sim::Torpedo& torpedo : world.torpedoes())
      collider(torpedo.pos(), s.torpedo.collision_radius);
    collider(star.pos(), s.star.radius);
  }
}

}  // namespace spacewar::app
