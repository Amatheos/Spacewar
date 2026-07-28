#pragma once

#include <memory>

#include "engine/core/input.h"

namespace se {

class Game;
struct EngineConfig {
  int width = 1280;
  int height = 720;
  const char* title = "Spacewar Engine";
};

class Engine {
 public:
  int Run(const EngineConfig&, std::unique_ptr<Game>);
  const InputState& input() const { return input_; }

 private:
  void UpdateInput(const std::array<bool, kKeyCount>& key_state);

  InputState input_;
};

}  // namespace se
