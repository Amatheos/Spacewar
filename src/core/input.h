#pragma once

namespace spacewar {

// ShipInput / GameInput are the neutral, per-frame snapshot of player intent
// that crosses the platform -> game boundary. Like GlProcLoader, this POD is a
// DTO neither side reaches *through*: platform fills it (Window::PollInput)
// from GLFW key state; game reads it in World::Update. GLFW never enters game.
//
// It lives in core -- not platform -- precisely so game can name the type
// without depending on platform, keeping the layering one-way. Polled, not
// event-driven: the fixed-step sim samples current key state each frame.
struct MenuInput {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool select = false;
  bool back = false;
  bool debug = false;
};

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

}  // namespace spacewar
