#pragma once

namespace spacewar::app {

// Player-facing options that aren't sim tunables (audio/video).
struct AppOptions {
  float master_volume = 1.0f;
  bool fullscreen = false;
};

bool LoadAppOptions(const char* path, AppOptions& out);
bool SaveAppOptions(const char* path, const AppOptions& in);

}  // namespace spacewar::app
