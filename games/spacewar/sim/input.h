#pragma once

namespace spacewar::sim {

struct ShipInput {
  bool rotate_left = false;
  bool rotate_right = false;
  bool thrust = false;
  bool fire = false;
  bool hyperspace = false;
};

struct GameInput {
  ShipInput players[2];
};

}  // namespace spacewar::sim
