#include "sim/world.h"

#include <cassert>
#include <vector>

#include "sim/settings.h"

namespace spacewar::sim {

using namespace se;

namespace {

Vec2 ShortestDelta(Vec2 a, Vec2 b) {
  Vec2 d = a - b;
  if (d.x > World::kHalfWidth)
    d.x -= 2.0f * World::kHalfWidth;
  else if (d.x < -World::kHalfWidth)
    d.x += 2.0f * World::kHalfWidth;
  if (d.y > World::kHalfHeight)
    d.y -= 2.0f * World::kHalfHeight;
  else if (d.y < -World::kHalfHeight)
    d.y += 2.0f * World::kHalfHeight;
  return d;
}

bool Overlap(Vec2 a, float ra, Vec2 b, float rb) {
  float r = ra + rb;
  return ShortestDelta(a, b).LengthSquared() < r * r;
}

}  // namespace

World::World(const SimSettings& settings)
    : settings_(&settings),
      ships_{Ship(settings_->spawn.needle_start,
                  {0.0f, settings_->spawn.orbit_speed}, kHalfPi),
             Ship(settings_->spawn.wedge_start,
                  {0.0f, -settings_->spawn.orbit_speed}, -kHalfPi)},
      respawn_timer_(settings_->match.respawn_sec),
      match_timer_(settings_->match.match_sec) {
  rng_.seed(settings_->rng_seed);
}

void World::Update(float dt, const GameInput& in) {
  events_.clear();
  switch (phase_) {
    case GamePhase::Countdown:
      CountdownStep(dt);
      break;
    case GamePhase::Simulating:
      SimulationStep(dt, in);
      break;
    case GamePhase::Settling:
      SettleStep(dt);
      break;
    case GamePhase::Finished:
      break;
  }
  ++tick_;
}

void World::SimulationStep(float dt, const GameInput& in) {
  for (std::size_t i = 0; i < ships_.size(); ++i) {
    Ship& ship = ships_[i];
    Hyperspace& hs = hyperspace_[i];

    // Process hyperspace jump animation
    if (hs.jumping()) {
      const float before = hs.timer;
      hs.timer -= dt;
      ship.ApplySpin(settings_->hyperspace.ship_spin_rate * dt);

      const float mid = settings_->hyperspace.anim_duration_sec / 2;
      if (before > mid && hs.timer <= mid) {
        std::uniform_int_distribution<int> roll(0, 99);
        if (roll(rng_) <
            settings_->hyperspace.base_fail_percent +
                hs.uses * settings_->hyperspace.fail_step_percent) {
          player_score_[i] -= settings_->hyperspace.suicide_penalty;
          ship.Kill();
        } else {
          std::uniform_real_distribution<float> xr(-kHalfWidth, kHalfWidth);
          std::uniform_real_distribution<float> yr(-kHalfHeight, kHalfHeight);
          std::uniform_real_distribution<float> ar(-kPi, kPi);
          std::uniform_real_distribution<float> sr(0.0f,
                                                   settings_->ship.max_speed);
          float dir = ar(rng_);
          ship = Ship({xr(rng_), yr(rng_)}, Vec2::FromAngle(dir) * sr(rng_),
                      ar(rng_));
          events_.push_back(
              {GameEvent::Kind::HyperspaceArrive, ship.pos(), ship.vel()});
        }
      }
      continue;
    }

    ship.Update(dt, star_.GravityAt(ship.pos(), settings_->star),
                settings_->ship, in.players[i], bounds_);

    // Fire torpedoes
    if (in.players[i].fire && ship.can_fire()) {
      Vec2 nose = Vec2::FromAngle(ship.angle());
      torpedoes_.emplace_back(
          ship.pos() + nose * settings_->ship.muzzle_offset,
          ship.vel() + nose * settings_->torpedo.launch_speed,
          static_cast<Player>(i), settings_->torpedo.life_sec);
      ship.ApplyImpulse(nose * -settings_->torpedo.recoil);
      ship.Fire(settings_->ship.fire_cooldown);
      player_score_[i] -= settings_->scoring.fire_cost;
      events_.push_back({GameEvent::Kind::TorpedoFired,
                         ship.pos() + nose * settings_->ship.muzzle_offset,
                         ship.vel() + nose * settings_->torpedo.launch_speed});
    }

    // Hyperspace jump: arm the timer and go on cooldown; the animation branch
    // above takes over from the next tick.
    if (hs.cooldown > 0.0f) hs.cooldown -= dt;
    if (in.players[i].hyperspace && hs.cooldown <= 0.0f) {
      hs.uses++;
      hs.timer = settings_->hyperspace.anim_duration_sec;
      events_.push_back(
          {GameEvent::Kind::HyperspaceDepart, ship.pos(), ship.vel()});
      hs.cooldown = settings_->hyperspace.cooldown_sec;
    }
  }

  // Torpedoes update
  for (Torpedo& torpedo : torpedoes_) {
    torpedo.Update(dt, star_.GravityAt(torpedo.pos(), settings_->star),
                   settings_->torpedo.max_speed, bounds_);
    if (!torpedo.alive())
      events_.push_back(
          {GameEvent::Kind::TorpedoExpired, torpedo.pos(), torpedo.vel()});
  }

  ResolveCollisions();

  std::erase_if(torpedoes_, [](const Torpedo& t) { return !t.alive(); });

  match_timer_ -= dt;
  bool round_over = !ships_[idx(Player::Needle)].alive() ||
                    !ships_[idx(Player::Wedge)].alive();

  if (match_timer_ <= 0.0f)
    PhaseTransition(GamePhase::Finished);
  else if (round_over)
    PhaseTransition(GamePhase::Settling);
}

void World::SettleStep(float dt) {
  settle_timer_ -= dt;
  if (settle_timer_ <= 0.0f) PhaseTransition(GamePhase::Countdown);
}

void World::CountdownStep(float dt) {
  const float before = respawn_timer_;
  respawn_timer_ -= dt;

  const int last_beep = static_cast<int>(settings_->match.respawn_sec);
  if (before >= settings_->match.respawn_sec)  // first tick: the top number
    events_.push_back({GameEvent::Kind::CountdownTick, {}, {}});
  for (int k = 1; k < last_beep; ++k)
    if (before > k && respawn_timer_ <= k)
      events_.push_back({GameEvent::Kind::CountdownTick, {}, {}});

  if (respawn_timer_ <= 0.0f) {
    respawn_timer_ = 0.0f;
    events_.push_back({GameEvent::Kind::CountdownGo, {}, {}});
    ResetBoard();
    PhaseTransition(GamePhase::Simulating);
  }
}

void World::PhaseTransition(GamePhase next) {
  assert(phase_ != next);
  if (next == GamePhase::Settling)
    settle_timer_ = settings_->match.settle_sec;
  else if (next == GamePhase::Countdown)
    respawn_timer_ = settings_->match.respawn_sec;
  phase_ = next;
}

void World::ResolveCollisions() {
  const float torpedo_r = settings_->torpedo.collision_radius;
  const float ship_r = settings_->ship.collision_radius;
  const float star_r = settings_->star.radius;

  auto collider_active = [&](const std::size_t s) -> bool {
    return ships_[s].alive() && !hyperspace_[s].jumping();
  };

  // Torpedoes expiring, flying into the star, or killing someone
  for (Torpedo& t : torpedoes_) {
    if (!t.alive()) continue;  // Expired earlier this tick
    if (Overlap(t.pos(), torpedo_r, star_.pos(), star_r)) {
      t.Expire();
      events_.push_back({GameEvent::Kind::TorpedoDetonated, t.pos(), t.vel()});
      continue;
    }
    for (std::size_t s = 0; s < ships_.size(); ++s)
      if (collider_active(s) &&
          Overlap(t.pos(), torpedo_r, ships_[s].pos(), ship_r)) {
        t.Expire();
        events_.push_back(
            {GameEvent::Kind::TorpedoDetonated, t.pos(), t.vel()});
        if (ships_[s].alive()) {
          ships_[s].Kill();
          player_score_[idx(t.owner())] += s == idx(t.owner())
                                               ? -settings_->scoring.self_kill
                                               : settings_->scoring.kill;
        }
        break;
      }
  }

  // Ships flying into the star
  for (std::size_t s = 0; s < ships_.size(); ++s)
    if (collider_active(s) &&
        Overlap(ships_[s].pos(), ship_r, star_.pos(), star_r)) {
      ships_[s].Kill();
      player_score_[s] -= settings_->scoring.star_suicide;
    }

  // Ships ramming into each other
  const Ship& a = ships_[idx(Player::Needle)];
  const Ship& b = ships_[idx(Player::Wedge)];
  if (collider_active(idx(Player::Needle)) &&
      collider_active(idx(Player::Wedge)) &&
      Overlap(a.pos(), ship_r, b.pos(), ship_r)) {
    ships_[idx(Player::Needle)].Kill();
    ships_[idx(Player::Wedge)].Kill();
  }

  for (std::size_t s = 0; s < ships_.size(); ++s)
    if (!ships_[s].alive())
      events_.push_back(
          {GameEvent::Kind::ShipExploded, ships_[s].pos(), ships_[s].vel()});
}

void World::ResetBoard() {
  ships_[idx(Player::Needle)] =
      Ship(settings_->spawn.needle_start, {0.0f, settings_->spawn.orbit_speed},
           kHalfPi);
  ships_[idx(Player::Wedge)] =
      Ship(settings_->spawn.wedge_start, {0.0f, -settings_->spawn.orbit_speed},
           -kHalfPi);
  torpedoes_.clear();
  for (Hyperspace& hs : hyperspace_) {
    hs.cooldown = 0.0f;
    hs.timer = 0.0f;
  }
}

}  // namespace spacewar::sim
