#pragma once

#include "game/world.h"
#include "render/font.h"
#include "render/frame.h"

namespace spacewar::app {

// Text is SDF, so the outline comes from the shader, not here.
class Hud {
 public:
  explicit Hud(const char* font_path);

  void BuildInto(render::Frame& frame, const game::World& world) const;

 private:
  render::Font font_;
};

}  // namespace spacewar::app
