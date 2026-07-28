#pragma once

#include "engine/core/math.h"

namespace spacewar::sim {

enum class Player;

class Torpedo {
 public:
  Torpedo(se::Vec2 pos, se::Vec2 vel, Player owner, float lifespan);

  void Update(float dt, se::Vec2 gravity, float max_speed,
              const se::Bounds& bounds);
  void Expire() { lifespan_ = 0.0f; }

  bool alive() const { return lifespan_ > 0.0f; }
  se::Vec2 pos() const { return pos_; }
  se::Vec2 vel() const { return vel_; }
  Player owner() const { return owner_; }

 private:
  se::Vec2 pos_;
  se::Vec2 vel_;
  float lifespan_ = 0.0f;
  Player owner_;
};

}  // namespace spacewar::sim
