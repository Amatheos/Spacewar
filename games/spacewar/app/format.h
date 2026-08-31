#pragma once

#include <string>

namespace spacewar::app {

// Seconds -> "M:SS" (negative clamps to 0).
inline std::string FormatClock(float seconds) {
  constexpr int kSecondsPerMinute = 60;
  constexpr int kZeroPadBelow = 10;
  int s = static_cast<int>(seconds < 0.0f ? 0.0f : seconds);
  std::string out = std::to_string(s / kSecondsPerMinute) + ":";
  int r = s % kSecondsPerMinute;
  if (r < kZeroPadBelow) out += '0';
  return out + std::to_string(r);
}

}  // namespace spacewar::app
