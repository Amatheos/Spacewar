#pragma once

#include <memory>

namespace se {

struct InputState;

namespace render {
struct Frame;
}

namespace audio {
class Engine;
}

namespace platform {
class Window;
}

struct Services {
  audio::Engine& audio;
  platform::Window& window;
};

class Game {
 public:
  virtual ~Game() = default;
  Game(const Game&) = delete;
  Game& operator=(const Game&) = delete;

  virtual bool Init(Services& svs) = 0;
  virtual void Update(float dt, const InputState& input) = 0;
  virtual void BuildFrame(render::Frame& frame) = 0;
  virtual bool WantsExit() const = 0;

  float update_hz() const { return update_hz_; }

 protected:
  explicit Game(float updates_per_s) : update_hz_(updates_per_s) {}

 private:
  const float update_hz_;
};

std::unique_ptr<Game> CreateGame();

}  // namespace se
