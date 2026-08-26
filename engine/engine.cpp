#include "engine/engine.h"

#include <algorithm>
#include <chrono>

#include "engine/audio/engine.h"
#include "engine/game.h"
#include "engine/platform/window.h"
#include "engine/render/renderer.h"
#include "profiler/profiler.h"

namespace se {

namespace {

class WindowDisplay : public Display {
 public:
  explicit WindowDisplay(platform::Window& w) : window_(w) {}
  void SetFullscreen(bool on) override { window_.SetFullscreen(on); }

 private:
  platform::Window& window_;
};

class NullDisplay : public Display {
 public:
  void SetFullscreen(bool) override {}
};

}  // namespace

bool Engine::InitGame(Game& game, audio::Engine& audio, Display& display) {
  Services svs{audio, display};
  return game.Init(svs);
}

int Engine::Run(const EngineConfig& en_cfg, const WindowConfig& win_cfg,
                std::unique_ptr<Game> game) {
  if (en_cfg.headless) {
    return RunHeadless(en_cfg.num_ticks, std::move(game));
  }
  return RunWindowed(win_cfg, std::move(game));
}

int Engine::RunHeadless(int num_ticks, std::unique_ptr<Game> game) {
  audio::Engine audio;
  NullDisplay null_display;
  if (!InitGame(*game, audio, null_display)) return 1;
  const float step = 1.0f / game->update_hz();

  for (int i = 0; i < num_ticks; i++) game->Update(step, input_);
  return 0;
}

int Engine::RunWindowed(const WindowConfig& win_cfg,
                        std::unique_ptr<Game> game) {
  platform::Window window(win_cfg.width, win_cfg.height, win_cfg.title);
  if (!window.ok()) return 1;

  std::unique_ptr<Game> owned = std::move(game);

  render::Renderer renderer;
  if (!renderer.Init(platform::Window::GlLoader())) return 1;
  window.SetResizeCallback([&](int w, int h) { renderer.Resize(w, h); });
  platform::Window::Extent fb = window.FramebufferSize();
  renderer.Resize(fb.width, fb.height);  // initial; later changes via callback

  audio::Engine audio;
  WindowDisplay window_display(window);
  if (!InitGame(*owned, audio, window_display)) return 1;
  const float step = 1.0f / owned->update_hz();

  using SteadyClock = std::chrono::steady_clock;
  SteadyClock::time_point prev = SteadyClock::now();
  float accumulator = 0.0f;

  while (!window.ShouldClose() && !owned->WantsExit()) {
    se::profiler::ScopedTimer tim("Frame");
    window.PollEvents();

    SteadyClock::time_point now = SteadyClock::now();
    float dt = std::chrono::duration<float>(now - prev).count();
    prev = now;

    UpdateInput(window.KeysDown());
    accumulator += std::min(dt, 0.25f);
    while (accumulator >= step) {
      owned->Update(step, input_);
      accumulator -= step;
    }

    render::Frame frame;
    frame.overlay_extent = renderer.overlay_extent();
    {
      se::profiler::ScopedTimer t("Game::BuildFrame");
      owned->BuildFrame(frame);
    }

    renderer.SetWorldView(frame.world_half.x, frame.world_half.y);
    renderer.BeginFrame(frame.clear_color);
    {
      se::profiler::ScopedTimer t("Renderer::Submit");
      renderer.Submit(frame);
    }

    {
      se::profiler::ScopedTimer t("SwapBuffers");
      window.SwapBuffers();
    }
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
