#pragma once

#include "engine/core/math.h"

namespace spacewar::sim {

struct StarSettings;

class Star {
 public:
  se::Vec2 GravityAt(se::Vec2 p, const StarSettings& settings) const;

  se::Vec2 pos() const { return pos_; }

 private:
  se::Vec2 pos_{0, 0};
};

}  // namespace spacewar::sim
