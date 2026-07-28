#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "engine/core/math.h"
#include "engine/render/frame.h"
#include "sim/world.h"

namespace spacewar::app {

class Effects {
 public:
  explicit Effects(std::uint32_t seed);
  void Consume(const std::vector<sim::GameEvent>& events);  // spawn bursts
  void Update(float dt);                           // integrate, age, cull
  void BuildInto(se::render::Frame& frame) const;  // additive Circles

 private:
  struct Particle {
    se::Vec2 pos, vel;
    float age, lifespan, radius;
    se::Color color;
  };
  std::vector<Particle> particles_;
  std::mt19937 rng_;
};

}  // namespace spacewar::app
