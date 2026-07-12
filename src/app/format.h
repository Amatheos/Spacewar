#pragma once

#include <string>

namespace spacewar::app {

// Seconds -> "M:SS" (negative clamps to 0).
inline std::string FormatClock(float seconds) {
  int s = static_cast<int>(seconds < 0.0f ? 0.0f : seconds);
  std::string out = std::to_string(s / 60) + ":";
  int r = s % 60;
  if (r < 10) out += '0';
  return out + std::to_string(r);
}

}  // namespace spacewar::app
