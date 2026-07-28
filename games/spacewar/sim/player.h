#pragma once

#include <cstddef>

namespace spacewar::sim {

// `Count` is the slot total (a sentinel, not a player) for sizing arrays and
// loops.
enum class Player { Needle, Wedge, Count };

inline constexpr std::size_t kPlayerCount =
    static_cast<std::size_t>(Player::Count);

constexpr std::size_t idx(Player p) { return static_cast<std::size_t>(p); }

}  // namespace spacewar::sim
