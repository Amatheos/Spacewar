#pragma once

#include <cstdint>
#include <vector>

#include "engine/core/math.h"

namespace se::render {

class Texture;

struct DrawCommand {
  enum class Kind { Polygon, Circle, Ring, Rect, RectFilled, Textured };

  Kind kind = Kind::Polygon;
  Vec2 pos{0, 0};
  Color color{1, 1, 1, 1};

  const std::vector<Vec2>* shape = nullptr;
  float angle = 0.0f;
  float scale = 1.0f;

  float radius = 0.0f;

  float thickness = 0.0f;

  Vec2 size{0, 0};

  const Texture* texture = nullptr;
  Vec2 uv_min{0, 0};
  Vec2 uv_max{1, 1};
};

struct Frame {
  Color clear_color{0.0f, 0.0f, 0.0f, 1.0f};
  Vec2 world_half{1.0f, 1.0f};
  Vec2 view_center{0.0f, 0.0f};
  std::uint64_t tick = 0;
  const Texture* background = nullptr;
  std::vector<DrawCommand> commands;
  std::vector<DrawCommand> overlay;
  Vec2 overlay_extent{1.0f, 1.0f};
};

}  // namespace se::render
