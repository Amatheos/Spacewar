#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "core/math.h"
#include "game/world.h"
#include "render/frame.h"

namespace spacewar::app {

class Effects {
 public:
  explicit Effects(std::uint32_t seed);
  void Consume(const std::vector<game::GameEvent>& events);  // spawn bursts
  void Update(float dt);                       // integrate, age, cull
  void BuildInto(render::Frame& frame) const;  // additive Circles

 private:
  struct Particle {
    Vec2 pos, vel;
    float age, lifespan, radius;
    Color color;
  };
  std::vector<Particle> particles_;
  std::mt19937 rng_;
};

}  // namespace spacewar::app
