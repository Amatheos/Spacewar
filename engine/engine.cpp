#include "engine/engine.h"

#include <algorithm>
#include <chrono>

#include "engine/audio/engine.h"
#include "engine/game.h"
#include "engine/platform/window.h"
#include "engine/render/renderer.h"

namespace se {

int Engine::Run(const EngineConfig& cfg, std::unique_ptr<Game> game) {
  platform::Window window(cfg.width, cfg.height, cfg.title);
  if (!window.ok()) return 1;

  render::Renderer renderer;
  if (!renderer.Init(platform::Window::GlLoader())) return 1;
  window.SetResizeCallback([&](int w, int h) { renderer.Resize(w, h); });
  platform::Window::Extent fb = window.FramebufferSize();
  renderer.Resize(fb.width, fb.height);  // initial; later changes via callback

  audio::Engine audio;
  Services svs{audio, window};
  if (!game->Init(svs)) return 1;

  using SteadyClock = std::chrono::steady_clock;
  SteadyClock::time_point prev = SteadyClock::now();
  const float step = 1.0f / game->update_hz();
  float accumulator = 0.0f;

  while (!window.ShouldClose() && !game->WantsExit()) {
    window.PollEvents();

    SteadyClock::time_point now = SteadyClock::now();
    float dt = std::chrono::duration<float>(now - prev).count();
    prev = now;

    UpdateInput(window.KeysDown());
    accumulator += std::min(dt, 0.25f);
    while (accumulator >= step) {
      game->Update(step, input_);
      accumulator -= step;
    }

    render::Frame frame;
    frame.overlay_extent = renderer.overlay_extent();
    game->BuildFrame(frame);

    renderer.SetWorldView(frame.world_half.x, frame.world_half.y);
    renderer.BeginFrame(frame.clear_color);
    renderer.Submit(frame);
    window.SwapBuffers();
  }
  return 0;
}

void Engine::UpdateInput(const std::array<bool, kKeyCount>& key_state) {
  for (std::size_t i = 0; i < kKeyCount; ++i) {
    input_.pressed[i] = key_state[i] && !input_.down[i];
    input_.released[i] = !key_state[i] && input_.down[i];
  }
  input_.down = key_state;
}

}  // namespace se
