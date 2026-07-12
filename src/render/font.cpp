#include "render/font.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

#include "render/texture.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace spacewar::render {
namespace {

constexpr int kAtlas = 1024;       // kAtlas x kAtlas, single channel (~1 MB)
constexpr float kBakePx = 128.0f;  // SDF sample size; upscales crisply
constexpr int kPadding = 6;        // distance-field spread per glyph, px
constexpr unsigned char kOnEdge =
    128;                        // byte value at the glyph edge (d = 0.5)
constexpr int kFirstChar = 32;  // space
constexpr int kCharCount = 95;  // printable ASCII 32..126

}  // namespace

Font::Font(const char* ttf_path) : first_char_(kFirstChar), bake_px_(kBakePx) {
  std::ifstream file(ttf_path, std::ios::binary);
  if (!file) {
    fprintf(stderr, "font: cannot open %s\n", ttf_path);
    return;
  }
  std::vector<unsigned char> ttf((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
  if (ttf.empty()) {
    fprintf(stderr, "font: %s is empty\n", ttf_path);
    return;
  }

  stbtt_fontinfo info;
  if (!stbtt_InitFont(&info, ttf.data(),
                      stbtt_GetFontOffsetForIndex(ttf.data(), 0))) {
    fprintf(stderr, "font: cannot parse %s\n", ttf_path);
    return;
  }

  const float scale = stbtt_ScaleForPixelHeight(&info, kBakePx);
  const float dist_scale = static_cast<float>(kOnEdge) / kPadding;

  std::vector<unsigned char> atlas(static_cast<std::size_t>(kAtlas) * kAtlas,
                                   0);
  glyphs_.resize(kCharCount);
  int pen_x = 0, pen_y = 0, row_h = 0;
  for (int i = 0; i < kCharCount; ++i) {
    int cp = kFirstChar + i;
    int advance = 0, lsb = 0;
    stbtt_GetCodepointHMetrics(&info, cp, &advance, &lsb);

    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* sdf = stbtt_GetCodepointSDF(
        &info, scale, cp, kPadding, kOnEdge, dist_scale, &w, &h, &xoff, &yoff);
    Glyph& g = glyphs_[i];
    g.xadvance = advance * scale;
    g.xoff = static_cast<float>(xoff);
    g.yoff = static_cast<float>(yoff);
    g.w = static_cast<float>(w);
    g.h = static_cast<float>(h);
    g.u0 = g.v0 = g.u1 = g.v1 = 0.0f;
    if (sdf == nullptr || w <= 0 || h <= 0) {
      if (sdf) stbtt_FreeSDF(sdf, nullptr);
      continue;  // blank glyph (space): advance only, nothing to draw
    }

    if (pen_x + w + 1 > kAtlas) {  // wrap to the next shelf
      pen_x = 0;
      pen_y += row_h + 1;
      row_h = 0;
    }
    if (pen_y + h > kAtlas) {
      fprintf(stderr, "font: SDF atlas overflow baking %s\n", ttf_path);
      stbtt_FreeSDF(sdf, nullptr);
      return;  // ok() stays false
    }
    for (int row = 0; row < h; ++row) {
      std::memcpy(
          &atlas[static_cast<std::size_t>(pen_y + row) * kAtlas + pen_x],
          &sdf[row * w], static_cast<std::size_t>(w));
    }
    g.u0 = static_cast<float>(pen_x) / kAtlas;
    g.v0 = static_cast<float>(pen_y) / kAtlas;
    g.u1 = static_cast<float>(pen_x + w) / kAtlas;
    g.v1 = static_cast<float>(pen_y + h) / kAtlas;
    pen_x += w + 1;
    row_h = std::max(row_h, h);
    stbtt_FreeSDF(sdf, nullptr);
  }

  texture_ = std::make_unique<Texture>(kAtlas, kAtlas, 1, atlas.data());
  if (!texture_->ok()) {
    texture_.reset();
    fprintf(stderr, "font: texture upload failed for %s\n", ttf_path);
  }
}

Font::~Font() = default;

void Font::AppendText(Frame& frame, std::string_view text, Vec2 baseline,
                      float height, const Color& color) const {
  if (!texture_) {
    return;
  }
  const float scale = height / bake_px_;
  float pen_x = baseline.x;
  for (char ch : text) {
    int c = static_cast<unsigned char>(ch);
    if (c < first_char_ ||
        c >= first_char_ + static_cast<int>(glyphs_.size())) {
      continue;
    }
    const Glyph& g = glyphs_[c - first_char_];
    // Space (and other blanks) has a zero-size quad but a real advance -- step
    // the pen without emitting an empty command.
    if (g.w > 0.0f && g.h > 0.0f) {
      DrawCommand cmd;
      cmd.kind = DrawCommand::Kind::Textured;
      cmd.pos = {pen_x + g.xoff * scale, baseline.y + g.yoff * scale};
      cmd.size = {g.w * scale, g.h * scale};
      cmd.uv_min = {g.u0, g.v0};
      cmd.uv_max = {g.u1, g.v1};
      cmd.texture = texture_.get();
      cmd.color = color;
      frame.overlay.push_back(cmd);
    }
    pen_x += g.xadvance * scale;
  }
}

float Font::MeasureWidth(std::string_view text, float height) const {
  if (!texture_) {
    return 0.0f;
  }
  const float scale = height / bake_px_;
  float width = 0.0f;
  for (char ch : text) {
    int c = static_cast<unsigned char>(ch);
    if (c < first_char_ ||
        c >= first_char_ + static_cast<int>(glyphs_.size())) {
      continue;
    }
    width += glyphs_[c - first_char_].xadvance * scale;
  }
  return width;
}

}  // namespace spacewar::render
