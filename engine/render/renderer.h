#pragma once

#include <memory>
#include <vector>

#include "engine/core/math.h"
#include "engine/core/types.h"
#include "engine/render/frame.h"
#include "engine/render/gl_context.h"

namespace se::render {

class Shader;

class Renderer {
 public:
  Renderer();
  ~Renderer();
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  bool Init(GlProcLoader loader);

  void SetWorldView(Vec2 center, Vec2 half_extent);

  void Resize(int width, int height);

  void BeginFrame(const Color& clear);

  void Submit(const Frame& frame);

  Vec2 overlay_extent() const { return overlay_extent_; }

 private:
  void DrawPolygon(const std::vector<Vec2>& pts, Vec2 pos, float angle,
                   float scale, const Color& color);

  void DrawCircle(Vec2 center, float radius, const Color& color);

  void DrawRing(Vec2 center, float radius, float thickness, const Color& color);

  void DrawRect(Vec2 pos, Vec2 size, float thickness, const Color& color);

  void DrawRectFilled(Vec2 pos, Vec2 size, const Color& color);

  void DrawTexturedQuad(Vec2 pos, Vec2 size, Vec2 uv_min, Vec2 uv_max,
                        const Texture& tex, const Color& color);

  void DrawBackground(const Texture& tex);

  void DrawVerts(unsigned int mode, const std::vector<Vec2>& pts,
                 const Mat3& model, const Color& color);

  void Execute(const DrawCommand& cmd);

  void RebuildWorldProjection();

 private:
  GlContext context_;
  std::unique_ptr<Shader> shader_;
  std::unique_ptr<Shader> textured_shader_;
  std::unique_ptr<Shader> image_shader_;
  unsigned int vao_ = 0;
  unsigned int vbo_ = 0;
  unsigned int tex_vao_ = 0;
  unsigned int tex_vbo_ = 0;
  Mat3 world_projection_;
  Mat3 screen_projection_;  // isotropic screen space for the overlay
  Vec2 overlay_extent_{1.0f, 1.0f};
  // Placeholder 16:9 view; SetWorldView replaces it before the first draw.
  float view_half_width_ = 100.0f;
  float view_half_height_ = 56.25f;
  Vec2 view_center_ = {0.0f, 0.0f};
  int fb_width_ = 1;
  int fb_height_ = 1;
};

}  // namespace se::render
