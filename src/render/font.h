#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "core/math.h"
#include "render/frame.h"

namespace spacewar::render {

class Texture;

class Font {
 public:
  explicit Font(const char* ttf_path);
  ~Font();
  Font(const Font&) = delete;
  Font& operator=(const Font&) = delete;

  bool ok() const { return static_cast<bool>(texture_); }

  void AppendText(Frame& frame, std::string_view text, Vec2 baseline,
                  float height, const Color& color) const;

  float MeasureWidth(std::string_view text, float height) const;

 private:
  // Atlas placement + metrics for one glyph, in baked-pixel units.
  struct Glyph {
    float u0, v0, u1, v1;  // atlas UV rect
    float xoff, yoff;      // pen -> quad top-left
    float w, h;            // quad size
    float xadvance;        // pen step to the next glyph
  };

  std::unique_ptr<Texture> texture_;
  std::vector<Glyph> glyphs_;
  int first_char_ = 32;
  float bake_px_ = 0.0f;
};

}  // namespace spacewar::render
