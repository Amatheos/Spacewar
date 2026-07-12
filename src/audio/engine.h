#pragma once

#include <memory>

namespace spacewar::audio {

// Owns the audio device and a high-level playback engine (miniaudio). One per
// app. Game-agnostic -- it plays sound files by path and knows nothing about
// the sim
class Engine {
 public:
  Engine();
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  // False if the device/engine failed to initialize; Play/SetVolume are then
  // no-ops (a machine with no audio device still runs the game silently).
  bool ok() const;

  // Fire-and-forget one-shot: decode `path` (WAV/MP3/FLAC) and play it once.
  // Safe to call every frame; a missing/!ok file is silently ignored.
  void PlayOnce(const char* path) const;

  // Master gain, linear (1.0 = unity). Clamped to >= 0.
  void SetVolume(float linear);

  void PlayLooped(const char* path);
  void StopPlayingLooped();

  // Continuous thruster loop, independent of the theme. Loaded once from `path`
  // on the first `on`, then toggled without re-decoding; `path` may be null on
  // later calls. A no-op when the state is unchanged, so it is cheap per frame.
  void SetThruster(bool on, const char* path);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace spacewar::audio
