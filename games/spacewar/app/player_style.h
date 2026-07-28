#pragma once

#include <array>
#include <vector>

#include "app/shapes.h"
#include "engine/core/math.h"
#include "sim/player.h"

namespace spacewar::app {

// Per-player presentation, indexed to input routing (0 = needle, 1 = wedge).
struct PlayerStyle {
  se::Color hull;                      // silhouette + HUD accent
  se::Color torpedo;                   // torpedo tint
  const std::vector<se::Vec2>* shape;  // silhouette outline
  se::Color exhaust;                   // thruster flame tint
  const std::vector<se::Vec2>* flame;  // thruster flame outline
  float engine;                        // ship-local x of the exhaust port
};

inline const std::array<PlayerStyle, sim::kPlayerCount> kPlayerStyles{{
    {{0.45f, 0.90f, 1.00f, 1.0f},
     {1.00f, 0.30f, 0.48f, 1.0f},
     &shapes::kNeedle,
     {1.00f, 0.55f, 0.12f, 1.0f},
     &shapes::kNeedleFlame,
     -1.0f},
    {{1.00f, 0.55f, 0.20f, 1.0f},
     {0.70f, 0.09f, 0.07f, 1.0f},
     &shapes::kWedge,
     {0.20f, 0.65f, 1.00f, 1.0f},
     &shapes::kWedgeFlame,
     -0.9f},
}};

}  // namespace spacewar::app
