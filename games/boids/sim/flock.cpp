#include "sim/flock.h"

#include <cmath>
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

bool InView(se::Vec2 fwd, float fwd_len2, se::Vec2 offset, float dist2,
            float fov_cos) {
  if (fwd_len2 <= 0.0f) return true;
  const float dot = fwd.Dot(offset);
  const float lhs = dot * dot;
  const float rhs = fov_cos * fov_cos * fwd_len2 * dist2;
  return fov_cos <= 0.0f ? (dot >= 0.0f || lhs < rhs)
                         : (dot > 0.0f && lhs > rhs);
}

}  // namespace

Flock::Flock(const FlockSettings& settings, se::Bounds bounds)
    : settings_(settings), bounds_(bounds), rng_(settings.rng_seed) {
  std::uniform_real_distribution<float> x(-bounds_.half_w, bounds_.half_w);
  std::uniform_real_distribution<float> y(-bounds_.half_h, bounds_.half_h);
  std::uniform_real_distribution<float> angle(-se::kPi, se::kPi);
  std::uniform_real_distribution<float> speed(settings_.min_speed,
                                              settings_.max_speed);
  std::uniform_real_distribution<float> offset(-settings_.speed_spread,
                                               settings_.speed_spread);

  boids_.reserve(settings_.count);
  for (int i = 0; i < settings_.count; ++i) {
    const float o = offset(rng_);
    boids_.push_back({{x(rng_), y(rng_)},
                      se::Vec2::FromAngle(angle(rng_)) * (speed(rng_) + o),
                      {},
                      o});
  }
}

void Flock::Update(float dt) {
  se::profiler::ScopedTimer t("Boids sim tick");
  const float r2 = settings_.perception_radius * settings_.perception_radius;
  const float fov_cos =
      std::cos(settings_.fov_degrees * 0.5f * se::kPi / 180.0f);

  for (std::size_t i = 0; i < boids_.size(); i++) {
    const Boid& a = boids_[i];

    se::Vec2 alignment = {};
    se::Vec2 cohesion = {};
    se::Vec2 separation = {};
    const float fwd_len2 = a.vel.LengthSquared();
    int seen = 0;
    int close = 0;

    for (std::size_t j = 0; j < boids_.size(); j++) {
      if (i == j) continue;
      const Boid& b = boids_[j];
      const se::Vec2 d = b.pos - a.pos;
      const float dist2 = d.LengthSquared();
      if (dist2 > r2 || dist2 <= 0.0f) continue;

      separation += d * (-1.0f / dist2);
      close++;

      if (!InView(a.vel, fwd_len2, d, dist2, fov_cos)) continue;
      alignment += b.vel;
      cohesion += b.pos;
      seen++;
    }

    const float max_force = settings_.max_force;
    const float max_speed = settings_.max_speed + a.speed_offset;

    se::Vec2 acc = {};
    if (close > 0) {
      acc += Steer(separation, a.vel, max_speed, max_force) *
             settings_.separation_weight;
    }
    if (seen > 0) {
      const float inv = 1.0f / seen;
      acc += Steer(alignment, a.vel, max_speed, max_force) *
             settings_.alignment_weight;
      acc += Steer(cohesion * inv - a.pos, a.vel, max_speed, max_force) *
             settings_.cohesion_weight;
    }

    const se::Vec2 repulsion =
        bounds_.RepulsionAt(a.pos, settings_.repulsion_margin);
    const float wall = repulsion.LengthSquared();
    if (wall > 0.0f) {
      acc += Steer(repulsion, a.vel, max_speed, max_force) *
             (settings_.repulsion_weight * wall);
    }

    boids_[i].acc = acc;
  }

  for (Boid& b : boids_) {
    b.vel = ClampSpeed(b.vel + b.acc * dt, settings_.min_speed + b.speed_offset,
                       settings_.max_speed + b.speed_offset);
    b.pos = bounds_.Clamp(b.pos + b.vel * dt);
  }
}

}  // namespace boids::sim
