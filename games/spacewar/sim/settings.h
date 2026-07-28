#pragma once

#include <cstdint>

#include "engine/core/math.h"

namespace spacewar::sim {

struct ShipSettings {
  float turn_rate;
  float thrust_accel;
  float max_speed;
  float fire_cooldown;
  float muzzle_offset;
  float collision_radius;
};

struct TorpedoSettings {
  float life_sec;
  float launch_speed;
  float max_speed;
  float recoil;
  float collision_radius;
};

struct StarSettings {
  float gravity;    // G*M in the softened inverse-square field
  float softening;  // |d| floor near the core (keeps accel finite)
  float radius;     // physical radius (collision + draw sizing)
};

struct SpawnSettings {
  se::Vec2 needle_start;
  se::Vec2 wedge_start;
  float orbit_speed;
};

struct MatchSettings {
  float respawn_sec;
  float match_sec;
  float settle_sec;  // frozen aftermath delay before the respawn countdown
};

struct ScoringSettings {
  int fire_cost;
  int kill;
  int self_kill;
  int star_suicide;
};

struct HyperspaceSettings {
  int base_fail_percent;    // self-destruct chance on the first jump
  int fail_step_percent;    // added chance per subsequent jump (rising risk)
  int suicide_penalty;      // charged on a malfunction death
  float cooldown_sec;       // minimum time between jumps
  float anim_duration_sec;  // jump animation duration
  float ship_spin_rate;     // how fast the ship spins during animation
};

struct SimSettings {
  ShipSettings ship;
  TorpedoSettings torpedo;
  StarSettings star;
  SpawnSettings spawn;
  MatchSettings match;
  ScoringSettings scoring;
  HyperspaceSettings hyperspace;
  std::uint32_t rng_seed;
};

bool LoadSimSettings(const char* path, SimSettings& out);

bool SaveSimSettings(const char* path, const SimSettings& in);

}  // namespace spacewar::sim
