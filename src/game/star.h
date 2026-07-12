#pragma once

#include "core/math.h"

namespace spacewar::game {
struct StarSettings;

class Star {
 public:
  Vec2 GravityAt(Vec2 p, const StarSettings& settings) const;

  Vec2 pos() const { return pos_; }

 private:
  Vec2 pos_{0, 0};
};

}  // namespace spacewar::game
