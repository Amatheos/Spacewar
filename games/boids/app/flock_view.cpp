#include "app/flock_view.h"

#include <cmath>

namespace boids::app {

using namespace se;

namespace {

// Nose at +X, matching Vec2::FromAngle's convention, so the heading angle
// applies directly. The notch at (-0.4, 0) makes it a dart rather than a
// triangle; a fan from the nose renders it correctly.
const std::vector<Vec2> kDart{
    {1.5f, 0.0f}, {-0.9f, 0.6f}, {-0.4f, 0.0f}, {-0.9f, -0.6f}};

constexpr float kBoidScale = 1.0f;
constexpr Color kBoidColor{0.62f, 0.84f, 1.0f, 0.95f};

}  // namespace

FlockView::FlockView() : shape_(kDart) {}

void FlockView::BuildInto(render::Frame& frame, const sim::Flock& flock) const {
  const std::vector<sim::Flock::Boid>& boids = flock.boids();
  frame.commands.reserve(frame.commands.size() + boids.size());

  for (const sim::Flock::Boid& boid : boids) {
    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Polygon;
    cmd.pos = boid.pos;
    cmd.angle = std::atan2(boid.vel.y, boid.vel.x);
    cmd.scale = kBoidScale;
    cmd.color = kBoidColor;
    cmd.shape = &shape_;
    frame.commands.push_back(cmd);
  }
}

}  // namespace boids::app
