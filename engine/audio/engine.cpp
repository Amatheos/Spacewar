#include "engine/audio/engine.h"

#include <miniaudio.h>

#include <algorithm>
#include <cstdio>

namespace se::audio {

// pImpl: the ma_engine lives here so <miniaudio.h> never reaches engine.h. The
// out-of-line destructor below is defined where Impl is complete, so
// unique_ptr<Impl> can delete it
struct Engine::Impl {
  ma_engine engine{};
  bool ready = false;
  ma_sound music{};
  bool music_loaded = false;
  ma_sound thruster{};
  bool thruster_loaded = false;
  bool thruster_on = false;
};

Engine::Engine() : impl_(std::make_unique<Impl>()) {
  ma_result result = ma_engine_init(nullptr, &impl_->engine);
  if (result != MA_SUCCESS) {
    std::fprintf(stderr, "audio: ma_engine_init failed (%d)\n", result);
    return;  // ok() stays false; Play/SetVolume become no-ops
  }
  impl_->ready = true;
}

Engine::~Engine() {
  if (impl_->thruster_loaded) ma_sound_uninit(&impl_->thruster);
  if (impl_->music_loaded) ma_sound_uninit(&impl_->music);
  if (impl_->ready) ma_engine_uninit(&impl_->engine);
}

bool Engine::ok() const { return impl_->ready; }

void Engine::PlayOnce(const char* path) const {
  if (!impl_->ready || path == nullptr) {
    return;
  }
  ma_engine_play_sound(&impl_->engine, path, nullptr);
}

void Engine::SetVolume(float linear) {
  if (!impl_->ready) {
    return;
  }
  ma_engine_set_volume(&impl_->engine, std::max(0.0f, linear));
}

void Engine::PlayLooped(const char* path) {
  if (!impl_->ready || path == nullptr) {
    return;
  }
  StopPlayingLooped();
  if (ma_sound_init_from_file(&impl_->engine, path, MA_SOUND_FLAG_STREAM,
                              nullptr, nullptr, &impl_->music) != MA_SUCCESS) {
    return;
  }
  impl_->music_loaded = true;
  ma_sound_set_looping(&impl_->music, MA_TRUE);
  ma_sound_start(&impl_->music);
}
void Engine::StopPlayingLooped() {
  if (!impl_->music_loaded) return;
  ma_sound_uninit(&impl_->music);
  impl_->music_loaded = false;
}

void Engine::SetThruster(bool on, const char* path) {
  if (!impl_->ready) return;
  if (on && !impl_->thruster_loaded) {
    if (path == nullptr) return;
    if (ma_sound_init_from_file(&impl_->engine, path, 0, nullptr, nullptr,
                                &impl_->thruster) != MA_SUCCESS) {
      return;
    }
    impl_->thruster_loaded = true;
    ma_sound_set_looping(&impl_->thruster, MA_TRUE);
  }
  if (!impl_->thruster_loaded || on == impl_->thruster_on) return;
  if (on)
    ma_sound_start(&impl_->thruster);
  else
    ma_sound_stop(&impl_->thruster);
  impl_->thruster_on = on;
}

}  // namespace se::audio
