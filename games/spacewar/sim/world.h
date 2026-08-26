#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <vector>

#include "engine/core/math.h"
#include "sim/events.h"
#include "sim/input.h"
#include "sim/player.h"
#include "sim/ship.h"
#include "sim/star.h"
#include "sim/torpedo.h"

namespace spacewar::sim {

struct SimSettings;

class World {
 public:
  enum class GamePhase { Countdown, Simulating, Settling, Finished };
  static constexpr float kHalfWidth = 100.0f;
  static constexpr float kHalfHeight = 56.25f;

  explicit World(const SimSettings& settings);

  void Update(float dt, const GameInput& in);

  const std::array<Ship, kPlayerCount>& ships() const { return ships_; }
  const Star& star() const { return star_; }
  const SimSettings& settings() const { return *settings_; }
  const std::vector<Torpedo>& torpedoes() const { return torpedoes_; }
  const std::vector<GameEvent>& events() const { return events_; }
  std::uint64_t tick() const { return tick_; }
  GamePhase game_phase() const { return phase_; }
  const std::array<int, kPlayerCount>& player_score() const {
    return player_score_;
  }
  float match_timer() const { return match_timer_; }
  float respawn_timer() const { return respawn_timer_; }
  float hyperspace_cooldown(Player p) const {
    return hyperspace_[idx(p)].cooldown;
  }
  float hyperspace_timer(Player p) const { return hyperspace_[idx(p)].timer; }

 private:
  void SimulationStep(float dt, const GameInput& in);
  void SettleStep(float dt);
  void CountdownStep(float dt);
  void PhaseTransition(GamePhase next);
  void ResolveCollisions();
  void ResetBoard();

  // The ctor binds it from a reference, so it is never null and needs no guard.
  const SimSettings* settings_;
  se::Bounds bounds_{kHalfWidth, kHalfHeight};
  struct Hyperspace {
    int uses = 0;
    float cooldown = 0.0f;
    float timer = 0.0f;
    bool jumping() const { return timer > 0.0f; }
  };

  std::array<Ship, kPlayerCount> ships_;
  std::array<int, kPlayerCount> player_score_{};
  std::array<Hyperspace, kPlayerCount> hyperspace_{};
  Star star_;
  std::vector<Torpedo> torpedoes_;
  std::vector<GameEvent> events_;
  std::uint64_t tick_ = 0;
  GamePhase phase_ = GamePhase::Countdown;
  float respawn_timer_ = 0.0f;
  float settle_timer_ = 0.0f;
  float match_timer_ = 0.0f;
  std::mt19937 rng_;
};

}  // namespace spacewar::sim
