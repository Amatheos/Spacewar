#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace se {

enum class Key : std::uint8_t {
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  Up,
  Down,
  Left,
  Right,
  Space,
  Enter,
  Escape,
  Tab,
  Backspace,
  LeftShift,
  RightShift,
  LeftCtrl,
  RightCtrl,
  LeftAlt,
  RightAlt,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  Count
};

inline constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);

struct InputState {
  std::array<bool, kKeyCount> down{};
  std::array<bool, kKeyCount> pressed{};
  std::array<bool, kKeyCount> released{};

  bool IsDown(Key k) const { return down[static_cast<std::size_t>(k)]; }
  bool WasPressed(Key k) const { return pressed[static_cast<std::size_t>(k)]; }
  bool WasReleased(Key k) const {
    return released[static_cast<std::size_t>(k)];
  }
};

}  // namespace se
