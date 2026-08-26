#pragma once

#include "engine/core/math.h"

namespace spacewar::sim {

// One-way report from the sim to the app layer: World appends these during
// Update, and the app reads them each frame to drive audio and particles.
struct GameEvent {
  enum class Kind {
    TorpedoFired,
    TorpedoDetonated,
    TorpedoExpired,
    ShipExploded,
    HyperspaceDepart,
    HyperspaceArrive,
    CountdownTick,
    CountdownGo
  };
  Kind kind;
  se::Vec2 pos;
  se::Vec2 vel;
};

}  // namespace spacewar::sim
