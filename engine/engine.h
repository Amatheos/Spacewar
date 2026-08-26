#pragma once

#include <memory>

#include "engine/core/input.h"

namespace se {

class Game;
class Display;

namespace audio {
class Engine;
}

struct EngineConfig {
  bool headless = false;
  int num_ticks = 0;
};

struct WindowConfig {
  int width = 1280;
  int height = 720;
  const char* title = "Spacewar Engine";
};

class Engine {
 public:
  int Run(const EngineConfig&, const WindowConfig&, std::unique_ptr<Game>);
  const InputState& input() const { return input_; }

 private:
  bool InitGame(Game&, audio::Engine&, Display&);
  int RunHeadless(int num_ticks, std::unique_ptr<Game>);
  int RunWindowed(const WindowConfig&,
                  std::unique_ptr<Game>);
  void UpdateInput(const std::array<bool, kKeyCount>& key_state);

  InputState input_;
};

}  // namespace se
