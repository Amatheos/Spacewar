#pragma once

#include "engine/render/font.h"
#include "engine/render/frame.h"
#include "sim/world.h"

namespace spacewar::app {

// Text is SDF, so the outline comes from the shader, not here.
class Hud {
 public:
  explicit Hud(const char* font_path);

  void BuildInto(se::render::Frame& frame, const sim::World& world) const;

 private:
  se::render::Font font_;
};

}  // namespace spacewar::app
