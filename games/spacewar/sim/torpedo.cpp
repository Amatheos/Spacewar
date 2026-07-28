#include "sim/torpedo.h"

namespace spacewar::sim {

using namespace se;

Torpedo::Torpedo(Vec2 pos, Vec2 vel, Player owner, float lifespan)
    : pos_(pos), vel_(vel), lifespan_(lifespan), owner_(owner) {}

void Torpedo::Update(float dt, Vec2 gravity, float max_speed,
                     const Bounds& bounds) {
  vel_ += gravity * dt;
  if (vel_.LengthSquared() > max_speed * max_speed) {
    vel_ = vel_.Normalized() * max_speed;  // cap slingshot runaway
  }
  pos_ = bounds.Wrap(pos_ + vel_ * dt);
  lifespan_ -= dt;
}

}  // namespace spacewar::sim
