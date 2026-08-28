#include "app/effects.h"

#include <array>
#include <cstddef>
#include <span>
#include <vector>

namespace spacewar::app {

using namespace se;

namespace {

struct Range {
  float min, max;
};

struct BurstSpec {
  int count;
  Range speed;
  Range life;
  float radius;
  std::span<const Color> palette;
};

constexpr std::array<Color, 3> kExplosionPalette{{{1.0f, 0.90f, 0.40f, 1.0f},
                                                  {1.0f, 0.50f, 0.12f, 1.0f},
                                                  {0.90f, 0.15f, 0.05f, 1.0f}}};
constexpr BurstSpec kShipExploded{.count = 28,
                                  .speed = {8.0f, 34.0f},
                                  .life = {0.5f, 1.0f},
                                  .radius = 1.1f,
                                  .palette = kExplosionPalette};

constexpr std::array<Color, 2> kDetonationPalette{
    {{1.0f, 0.85f, 0.40f, 1.0f}, {1.0f, 0.40f, 0.10f, 1.0f}}};
constexpr BurstSpec kTorpedoDetonated{.count = 10,
                                      .speed = {6.0f, 22.0f},
                                      .life = {0.3f, 0.6f},
                                      .radius = 0.7f,
                                      .palette = kDetonationPalette};

constexpr std::array<Color, 2> kFizzlePalette{
    {{0.60f, 0.60f, 0.66f, 1.0f}, {0.40f, 0.40f, 0.45f, 1.0f}}};
constexpr BurstSpec kTorpedoExpired{.count = 6,
                                    .speed = {2.0f, 8.0f},
                                    .life = {0.3f, 0.6f},
                                    .radius = 0.6f,
                                    .palette = kFizzlePalette};

constexpr std::array<Color, 2> kHyperspacePalette{
    {{0.60f, 0.50f, 1.0f, 1.0f}, {0.85f, 0.70f, 1.0f, 1.0f}}};
constexpr BurstSpec kHyperspaceJump{.count = 20,
                                    .speed = {10.0f, 26.0f},
                                    .life = {0.3f, 0.5f},
                                    .radius = 0.7f,
                                    .palette = kHyperspacePalette};

// Fraction of its radius a particle sheds over a full lifetime.
constexpr float kShrinkOverLife = 0.5f;

}  // namespace

Effects::Effects(std::uint32_t seed) : rng_(seed) {}

void Effects::Consume(const std::vector<sim::GameEvent>& events) {
  using Kind = sim::GameEvent::Kind;

  auto burst = [&](Vec2 origin, Vec2 drift, const BurstSpec& spec) {
    std::uniform_real_distribution<float> angle(-kPi, kPi);
    std::uniform_real_distribution<float> speed(spec.speed.min, spec.speed.max);
    std::uniform_real_distribution<float> life(spec.life.min, spec.life.max);
    std::uniform_int_distribution<std::size_t> hue(0, spec.palette.size() - 1);

    for (int i = 0; i < spec.count; ++i) {
      Particle p;
      p.pos = origin;
      p.vel = Vec2::FromAngle(angle(rng_)) * speed(rng_) + drift;
      p.lifespan = life(rng_);
      p.radius = spec.radius;
      p.color = spec.palette[hue(rng_)];
      particles_.push_back(p);
    }
  };

  for (const sim::GameEvent& e : events) {
    switch (e.kind) {
      case Kind::ShipExploded:
        burst(e.pos, e.vel, kShipExploded);
        break;
      case Kind::TorpedoDetonated:
        burst(e.pos, e.vel, kTorpedoDetonated);
        break;
      case Kind::TorpedoExpired:
        burst(e.pos, e.vel, kTorpedoExpired);
        break;
      case Kind::HyperspaceDepart:
      case Kind::HyperspaceArrive:
        burst(e.pos, e.vel, kHyperspaceJump);
        break;
      case Kind::TorpedoFired:
      case Kind::CountdownTick:
      case Kind::CountdownGo:
        break;
    }
  }
}

void Effects::Update(float dt) {
  for (Particle& p : particles_) {
    p.pos += p.vel * dt;  // linear drift; no gravity/drag (purely cosmetic)
    p.age += dt;
  }

  std::erase_if(particles_,
                [](const Particle& p) { return p.age >= p.lifespan; });
}

void Effects::BuildInto(render::Frame& frame) const {
  frame.commands.reserve(frame.commands.size() + particles_.size());
  for (const Particle& p : particles_) {
    const float t = p.age / p.lifespan;  // 0 at spawn, 1 at death

    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Circle;
    cmd.pos = p.pos;
    cmd.radius = p.radius * (1.0f - kShrinkOverLife * t);
    cmd.color = p.color;
    cmd.color.a = 1.0f - t;  // linear fade-out
    frame.commands.push_back(cmd);
  }
}

}  // namespace spacewar::app
