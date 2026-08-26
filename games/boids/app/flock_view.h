#pragma once

#include <vector>

#include "engine/core/math.h"
#include "engine/render/frame.h"
#include "sim/flock.h"

namespace boids::app {

class FlockView {
 public:
  FlockView();

  void BuildInto(se::render::Frame& frame, const sim::Flock& flock) const;

 private:
  std::vector<se::Vec2> shape_;
};

}  // namespace boids::app
