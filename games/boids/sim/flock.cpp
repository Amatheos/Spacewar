#include "sim/flock.h"

#include <cstddef>

#include "engine/profiler/profiler.h"

namespace boids::sim {
namespace {

se::Vec2 ClampMagnitude(se::Vec2 v, float max) {
  return v.LengthSquared() > max * max ? v.Normalized() * max : v;
}

se::Vec2 Steer(se::Vec2 desired_dir, se::Vec2 vel, float max_speed,
               float max_force) {
  return ClampMagnitude(desired_dir.Normalized() * max_speed - vel, max_force);
}

// A stalled boid has no heading to restore, so zero stays zero.
se::Vec2 ClampSpeed(se::Vec2 v, float min, float max) {
  const float len2 = v.LengthSquared();
  if (len2 > max * max) return v.Normalized() * max;
  if (len2 > 0.0f && len2 < min * min) return v.Normalized() * min;
  return v;
}

}  // namespace

Flock::Flock(const FlockSettings& settings, se::Bounds bounds)
    : settings_(settings), bounds_(bounds), rng_(settings.rng_seed) {
  std::uniform_real_distribution<float> x(-bounds_.half_w, bounds_.half_w);
  std::uniform_real_distribution<float> y(-bounds_.half_h, bounds_.half_h);
  std::uniform_real_distribution<float> angle(-se::kPi, se::kPi);
  std::uniform_real_distribution<float> speed(settings_.min_speed,
                                              settings_.max_speed);

  boids_.reserve(settings_.count);
  for (int i = 0; i < settings_.count; ++i) {
    boids_.push_back(
        {{x(rng_), y(rng_)}, se::Vec2::FromAngle(angle(rng_)) * speed(rng_)});
  }
}

void Flock::Update(float dt) {
  se::profiler::ScopedTimer t("Boids sim tick");
  const float r2 = settings_.perception_radius * settings_.perception_radius;

  for (std::size_t i = 0; i < boids_.size(); i++) {
    const Boid& a = boids_[i];

    se::Vec2 alignment = {};
    se::Vec2 cohesion = {};
    se::Vec2 separation = {};
    int neighbours = 0;

    for (std::size_t j = 0; j < boids_.size(); j++) {
      if (i == j) continue;
      const Boid& b = boids_[j];
      const float dist2 = se::DistSquared(a.pos, b.pos);
      if (dist2 > r2 || dist2 <= 0.0f) continue;

      alignment += b.vel;
      cohesion += b.pos;
      separation += (a.pos - b.pos) * (1.0f / dist2);
      neighbours++;
    }

    if (neighbours == 0) {
      boids_[i].acc = {};
      continue;
    }

    const float inv = 1.0f / neighbours;
    const float max_force = settings_.max_force;
    const float max_speed = settings_.max_speed;
    alignment = Steer(alignment, a.vel, max_speed, max_force);
    cohesion = Steer(cohesion * inv - a.pos, a.vel, max_speed, max_force);
    separation = Steer(separation, a.vel, max_speed, max_force);

    boids_[i].acc = alignment * settings_.alignment_weight +
                    cohesion * settings_.cohesion_weight +
                    separation * settings_.separation_weight;
  }

  for (Boid& b : boids_) {
    b.vel = ClampSpeed(b.vel + b.acc * dt, settings_.min_speed,
                       settings_.max_speed);
    b.pos = bounds_.Wrap(b.pos + b.vel * dt);
  }
}

}  // namespace boids::sim
