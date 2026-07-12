#pragma once

#include "game/world.h"
#include "render/frame.h"

namespace spacewar::app {

// The presentation layer.
class GameView {
 public:
  void BuildInto(render::Frame& frame, const game::World& world,
                 bool show_colliders) const;
};

}  // namespace spacewar::app
