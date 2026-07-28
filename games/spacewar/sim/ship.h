#pragma once

#include "engine/core/math.h"

namespace spacewar::sim {

struct ShipInput;
struct ShipSettings;

class Ship {
 public:
  Ship(se::Vec2 pos, se::Vec2 vel, float angle);

  void Update(float dt, se::Vec2 gravity, const ShipSettings& settings,
              const ShipInput& in, const se::Bounds& bounds);

  se::Vec2 pos() const { return pos_; }
  se::Vec2 vel() const { return vel_; }
  float angle() const { return heading_; }

  void ApplyImpulse(se::Vec2 dv) { vel_ += dv; }
  void ApplySpin(float da) { heading_ += da; }

  bool can_fire() const { return fire_cooldown_ <= 0.0f; }
  void Fire(float cooldown) { fire_cooldown_ = cooldown; }

  bool alive() const { return !dead_; }
  void Kill() { dead_ = true; }

  bool thrusting() const { return thrusting_; }

 private:
  se::Vec2 pos_;
  se::Vec2 vel_;
  float heading_;
  float fire_cooldown_ = 0.0f;
  bool dead_ = false;
  bool thrusting_ = false;
};

}  // namespace spacewar::sim
