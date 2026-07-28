#pragma once

#include "engine/render/frame.h"
#include "sim/world.h"

namespace spacewar::app {

// The presentation layer.
class GameView {
 public:
  void BuildInto(se::render::Frame& frame, const sim::World& world,
                 bool show_colliders) const;
};

}  // namespace spacewar::app
