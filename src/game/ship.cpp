#include "game/ship.h"

#include "core/input.h"
#include "game/settings.h"

namespace spacewar::game {

Ship::Ship(Vec2 pos, Vec2 vel, float angle)
    : pos_(pos), vel_(vel), heading_(angle) {}

void Ship::Update(float dt, Vec2 gravity, const ShipSettings& settings,
                  const ShipInput& in, const Bounds& bounds) {
  if (in.rotate_left) heading_ += settings.turn_rate * dt;   // CCW
  if (in.rotate_right) heading_ -= settings.turn_rate * dt;  // CW

  thrusting_ = in.thrust;
  Vec2 accel = gravity;
  if (in.thrust) accel += Vec2::FromAngle(heading_) * settings.thrust_accel;

  vel_ += accel * dt;
  if (vel_.LengthSquared() > settings.max_speed * settings.max_speed) {
    vel_ = vel_.Normalized() * settings.max_speed;  // cap slingshot runaway
  }
  pos_ = bounds.Wrap(pos_ + vel_ * dt);

  if (fire_cooldown_ > 0.0f) fire_cooldown_ -= dt;
}

}  // namespace spacewar::game
