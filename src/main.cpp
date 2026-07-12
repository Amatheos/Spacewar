#include <chrono>

#include "app/app.h"
#include "app/options.h"
#include "core/math.h"
#include "game/settings.h"
#include "game/world.h"
#include "platform/window.h"
#include "render/frame.h"
#include "render/renderer.h"

namespace {
constexpr spacewar::Color kBackground{0.02f, 0.02f, 0.05f, 1.0f};
}  // namespace

int main() {
  using namespace spacewar;

  platform::Window window(1280, 720, "Spacewar-OOP");
  if (!window.ok()) return 1;

  render::Renderer renderer;
  if (!renderer.Init(platform::Window::GlLoader())) return 1;
  renderer.SetWorldView(game::World::kHalfWidth, game::World::kHalfHeight);
  window.SetResizeCallback([&](int w, int h) { renderer.Resize(w, h); });
  platform::Window::Extent fb = window.FramebufferSize();
  renderer.Resize(fb.width, fb.height);  // initial; later changes via callback

  game::SimSettings settings;
  if (!game::LoadSimSettings(SPACEWAR_SETTINGS_PATH, settings)) return 1;
  app::AppOptions options;
  app::LoadAppOptions(SPACEWAR_OPTIONS_PATH, options);

  app::App app(settings, options);
  app.SetFullscreenHandler([&](bool on) { window.SetFullscreen(on); });
  window.SetFullscreen(options.fullscreen);

  using SteadyClock = std::chrono::steady_clock;
  SteadyClock::time_point prev = SteadyClock::now();

  while (!window.ShouldClose() && !app.should_quit()) {
    window.PollEvents();

    SteadyClock::time_point now = SteadyClock::now();
    float dt = std::chrono::duration<float>(now - prev).count();
    prev = now;

    app.Update(dt, window.PollInput(), window.PollMenu());

    render::Frame frame;
    frame.overlay_extent = renderer.overlay_extent();
    app.BuildInto(frame);

    renderer.BeginFrame(kBackground);
    renderer.Submit(frame);
    window.SwapBuffers();
  }
  return 0;
}
