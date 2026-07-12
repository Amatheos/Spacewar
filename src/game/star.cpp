#include "game/star.h"

#include <cmath>

#include "game/settings.h"

namespace spacewar::game {

Vec2 Star::GravityAt(Vec2 p, const StarSettings& settings) const {
  Vec2 d = pos_ - p;  // toward the star
  // a = G*M * d / (|d|^2 + eps^2)^(3/2): softened inverse-square, finite at
  // d=0.
  float soft = d.LengthSquared() + settings.softening * settings.softening;
  return d * (settings.gravity / (soft * std::sqrt(soft)));
}

}  // namespace spacewar::game
