#include "app/effects.h"

#include <algorithm>
#include <initializer_list>

spacewar::app::Effects::Effects(std::uint32_t seed) : rng_(seed) {}

void spacewar::app::Effects::Consume(
    const std::vector<game::GameEvent>& events) {
  using Kind = game::GameEvent::Kind;

  auto burst = [&](Vec2 origin, Vec2 drift, int count, float speed_min,
                   float speed_max, float life_min, float life_max,
                   float radius, std::initializer_list<Color> palette) {
    std::uniform_real_distribution<float> angle(-kPi, kPi);
    std::uniform_real_distribution<float> speed(speed_min, speed_max);
    std::uniform_real_distribution<float> life(life_min, life_max);
    std::uniform_int_distribution<std::size_t> hue(0, palette.size() - 1);

    for (int i = 0; i < count; ++i) {
      Particle p;
      p.pos = origin;
      p.vel = Vec2::FromAngle(angle(rng_)) * speed(rng_) + drift;
      p.age = 0.0f;
      p.lifespan = life(rng_);
      p.radius = radius;
      p.color = palette.begin()[hue(rng_)];
      particles_.push_back(p);
    }
  };

  for (const game::GameEvent& e : events) {
    switch (e.kind) {
      // A ship dies: a big, hot bloom of yellow -> orange -> red that stands
      // in for the hull that just vanished.
      case Kind::ShipExploded:
        burst(e.pos, e.vel, 28, 8.0f, 34.0f, 0.5f, 1.0f, 1.1f,
              {{1.0f, 0.90f, 0.40f, 1.0f},
               {1.0f, 0.50f, 0.12f, 1.0f},
               {0.90f, 0.15f, 0.05f, 1.0f}});
        break;
      // A torpedo strikes a ship or the star: a smaller, quicker fiery pop.
      case Kind::TorpedoDetonated:
        burst(e.pos, e.vel, 10, 6.0f, 22.0f, 0.3f, 0.6f, 0.7f,
              {{1.0f, 0.85f, 0.40f, 1.0f}, {1.0f, 0.40f, 0.10f, 1.0f}});
        break;
      // A torpedo simply ages out: a weak grey fizzle -- "nothing happened".
      case Kind::TorpedoExpired:
        burst(e.pos, e.vel, 6, 2.0f, 8.0f, 0.3f, 0.6f, 0.6f,
              {{0.60f, 0.60f, 0.66f, 1.0f}, {0.40f, 0.40f, 0.45f, 1.0f}});
        break;
      // A jump at either endpoint: a violet shimmer.
      case Kind::HyperspaceDepart:
      case Kind::HyperspaceArrive:
        burst(e.pos, e.vel, 20, 10.0f, 26.0f, 0.3f, 0.5f, 0.7f,
              {{0.60f, 0.50f, 1.0f, 1.0f}, {0.85f, 0.70f, 1.0f, 1.0f}});
        break;
      case Kind::TorpedoFired:
      case Kind::CountdownTick:
      case Kind::CountdownGo:
        break;
    }
  }
}

void spacewar::app::Effects::Update(float dt) {
  for (Particle& p : particles_) {
    p.pos += p.vel * dt;  // linear drift; no gravity/drag (purely cosmetic)
    p.age += dt;
  }

  particles_.erase(
      std::remove_if(particles_.begin(), particles_.end(),
                     [](const Particle& p) { return p.age >= p.lifespan; }),
      particles_.end());
}

void spacewar::app::Effects::BuildInto(render::Frame& frame) const {
  for (const Particle& p : particles_) {
    render::DrawCommand cmd;
    cmd.kind = render::DrawCommand::Kind::Circle;
    cmd.pos = p.pos;
    float t = p.age / p.lifespan;
    cmd.radius = p.radius * (1 - (0.5 * t));  // Shrink particle with age
    cmd.color = p.color;
    cmd.color.a = 1 - t;  // Fade
    frame.commands.push_back(cmd);
  }
}
