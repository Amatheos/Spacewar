#pragma once

#include "core/math.h"

namespace spacewar::game {

enum class Player;

class Torpedo {
 public:
  Torpedo(Vec2 pos, Vec2 vel, Player owner, float lifespan);

  void Update(float dt, Vec2 gravity, float max_speed, const Bounds& bounds);
  void Expire() { lifespan_ = 0.0f; }

  bool alive() const { return lifespan_ > 0.0f; }
  Vec2 pos() const { return pos_; }
  Vec2 vel() const { return vel_; }
  Player owner() const { return owner_; }

 private:
  Vec2 pos_;
  Vec2 vel_;
  float lifespan_ = 0.0f;
  Player owner_;
};

}  // namespace spacewar::game
