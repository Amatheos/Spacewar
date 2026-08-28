#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "engine/core/math.h"

namespace boids::sim {

struct FlockSettings {
  int count = 500;
  float perception_radius = 6.0f;
  float min_speed = 12.0f;
  float max_speed = 30.0f;
  float max_force = 60.0f;
  float repulsion_margin = 20.0f;
  float repulsion_weight = 2.0f;
  float speed_spread = 4.0f;
  float fov_degrees = 270.0f;
  float separation_weight = 1.5f;
  float alignment_weight = 1.0f;
  float cohesion_weight = 1.2f;
  std::uint32_t rng_seed = 1234;
};

class Flock {
 public:
  struct Boid {
    se::Vec2 pos{};
    se::Vec2 vel{};
    se::Vec2 acc{};
    float speed_offset = 0.0f;
  };

  Flock(const FlockSettings& settings, se::Bounds bounds);

  void Update(float dt);
  const std::vector<Boid>& boids() const { return boids_; }

 private:
  FlockSettings settings_;
  se::Bounds bounds_;
  std::vector<Boid> boids_;
  std::mt19937 rng_;
};
}  // namespace boids::sim
