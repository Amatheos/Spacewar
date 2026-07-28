#include "engine/render/renderer.h"

#include <glad/glad.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "engine/render/shader.h"
#include "engine/render/texture.h"

namespace se::render {
namespace {

// Segment count of the unit fan DrawCircle uploads; plenty for a smooth star.
constexpr int kCircleSegments = 48;

// One vertex of a textured quad: screen/local position + atlas UV.
struct TexVertex {
  Vec2 pos;
  Vec2 uv;
};

constexpr const char* kVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat3 uModel;
uniform mat3 uProjection;
void main() {
  gl_Position = vec4((uProjection * uModel * vec3(aPos, 1.0)).xy, 0.0, 1.0);
}
)";

constexpr const char* kFragmentShader = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main() {
  FragColor = uColor;
}
)";

// Textured pipeline: same transform as the solid one, plus a UV attribute the
// fragment stage uses to sample the atlas.
constexpr const char* kTexVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
uniform mat3 uModel;
uniform mat3 uProjection;
out vec2 vUV;
void main() {
  vUV = aUV;
  gl_Position = vec4((uProjection * uModel * vec3(aPos, 1.0)).xy, 0.0, 1.0);
}
)";

constexpr const char* kTexFragmentShader = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform vec4 uColor;     // fill colour
uniform sampler2D uTex;  // SDF atlas: .r is distance, 0.5 at the glyph edge
void main() {
  // Signed-distance text: reconstruct a crisp edge at any scale. fwidth gives a
  // ~1px screen-space transition (resolution-independent antialiasing). The
  // outline is free -- a second threshold a little outside the edge, filled
  // black, with the colour crossfading to uColor across the true edge.
  float d = texture(uTex, vUV).r;
  float aa = fwidth(d);
  float fill = smoothstep(0.5 - aa, 0.5 + aa, d);      // 1 inside the glyph
  float edge = 0.5 - 0.32;                             // outline reaches to here
  float shape = smoothstep(edge - aa, edge + aa, d);   // 1 inside glyph+outline
  vec3 rgb = mix(vec3(0.72, 0.15, 1.0), uColor.rgb, fill);  // purple outline -> fill
  FragColor = vec4(rgb, shape * uColor.a);
}
)";

// Full-colour images (background, sprites): sample RGBA straight, tint by
// uColor. Distinct from the SDF text shader above, which reads only .r.
constexpr const char* kImageFragmentShader = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform vec4 uColor;
uniform sampler2D uTex;
void main() {
  FragColor = texture(uTex, vUV) * uColor;
}
)";

}  // namespace

Renderer::Renderer() = default;

Renderer::~Renderer() {
  if (context_.loaded()) {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &tex_vbo_);
    glDeleteVertexArrays(1, &tex_vao_);
  }
}

bool Renderer::Init(GlProcLoader loader) {
  if (!context_.Load(loader)) {
    return false;
  }
  shader_ = std::make_unique<Shader>(kVertexShader, kFragmentShader);
  if (!shader_->ok()) return false;

  textured_shader_ =
      std::make_unique<Shader>(kTexVertexShader, kTexFragmentShader);
  if (!textured_shader_->ok()) return false;
  textured_shader_->Use();
  textured_shader_->SetInt("uTex", 0);

  image_shader_ =
      std::make_unique<Shader>(kTexVertexShader, kImageFragmentShader);
  if (!image_shader_->ok()) return false;
  image_shader_->Use();
  image_shader_->SetInt("uTex", 0);

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
  glEnableVertexAttribArray(0);

  glGenVertexArrays(1, &tex_vao_);
  glGenBuffers(1, &tex_vbo_);
  glBindVertexArray(tex_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TexVertex),
                        (void*)offsetof(TexVertex, pos));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexVertex),
                        (void*)offsetof(TexVertex, uv));
  glEnableVertexAttribArray(1);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  return true;
}

void Renderer::SetWorldView(float half_width, float half_height) {
  view_half_width_ = half_width;
  view_half_height_ = half_height;
  RebuildWorldProjection();
}

// Expands the requested world extents along one axis so the window's aspect
// ratio never stretches them: extra window space shows more world instead.
void Renderer::RebuildWorldProjection() {
  float window_aspect =
      static_cast<float>(fb_width_) / static_cast<float>(fb_height_);
  float world_aspect = view_half_width_ / view_half_height_;
  float half_w = view_half_width_;
  float half_h = view_half_height_;
  if (window_aspect >= world_aspect) {
    half_w = half_h * window_aspect;
  } else {
    half_h = half_w / window_aspect;
  }
  world_projection_ = Mat3::Ortho(-half_w, half_w, -half_h, half_h);
}

void Renderer::Resize(int width, int height) {
  context_.Resize(width, height);
  if (width <= 0 || height <= 0) {
    return;
  }
  fb_width_ = width;
  fb_height_ = height;
  RebuildWorldProjection();

  // Overlay: isotropic screen space normalized by the shorter side, so one unit
  // is the same pixel size on both axes (text keeps its proportions) in either
  // orientation. The longer axis then spans 0..(long/short).
  float ref = static_cast<float>(std::min(width, height));
  overlay_extent_ = {static_cast<float>(width) / ref,
                     static_cast<float>(height) / ref};
  screen_projection_ =
      Mat3::Ortho(0.0f, overlay_extent_.x, overlay_extent_.y, 0.0f);
}

void Renderer::BeginFrame(const Color& clear) {
  glClearColor(clear.r, clear.g, clear.b, clear.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Submit(const Frame& frame) {
  if (frame.background != nullptr) {
    // Opaque image under the whole scene: standard alpha blend for this draw,
    // then restore the world's additive blend.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DrawBackground(*frame.background);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  }

  shader_->Use();
  shader_->SetMat3("uProjection", world_projection_);
  for (const DrawCommand& cmd : frame.commands) Execute(cmd);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  shader_->Use();
  shader_->SetMat3("uProjection", screen_projection_);
  for (const DrawCommand& cmd : frame.overlay) Execute(cmd);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void Renderer::Execute(const DrawCommand& cmd) {
  switch (cmd.kind) {
    case DrawCommand::Kind::Polygon:
      DrawPolygon(*cmd.shape, cmd.pos, cmd.angle, cmd.scale, cmd.color);
      break;
    case DrawCommand::Kind::Circle:
      DrawCircle(cmd.pos, cmd.radius, cmd.color);
      break;
    case DrawCommand::Kind::Ring:
      DrawRing(cmd.pos, cmd.radius, cmd.thickness, cmd.color);
      break;
    case DrawCommand::Kind::Rect:
      DrawRect(cmd.pos, cmd.size, cmd.color);
      break;
    case DrawCommand::Kind::Textured:
      DrawTexturedQuad(cmd.pos, cmd.size, cmd.uv_min, cmd.uv_max, *cmd.texture,
                       cmd.color);
      break;
  }
}

void Renderer::DrawPolygon(const std::vector<Vec2>& pts, Vec2 pos, float angle,
                           float scale, const Color& color) {
  Mat3 model = Mat3::Translate(pos.x, pos.y) * Mat3::Rotate(angle) *
               Mat3::Scale(scale, scale);
  DrawVerts(GL_TRIANGLE_FAN, pts, model, color);
}

void Renderer::DrawCircle(Vec2 center, float radius, const Color& color) {
  // Unit triangle fan (center + rim), positioned by the model transform.
  std::vector<Vec2> fan(kCircleSegments + 2);
  fan[0] = {0, 0};
  for (int i = 0; i <= kCircleSegments; ++i) {
    fan[i + 1] =
        Vec2::FromAngle(static_cast<float>(i) / kCircleSegments * kTwoPi);
  }
  Mat3 model =
      Mat3::Translate(center.x, center.y) * Mat3::Scale(radius, radius);
  DrawVerts(GL_TRIANGLE_FAN, fan, model, color);
}

void Renderer::DrawRing(Vec2 center, float radius, float thickness,
                        const Color& color) {
  const float half = thickness * 0.5f;
  const float inner = std::max(0.0f, radius - half);
  const float outer = radius + half;
  std::vector<Vec2> band;
  band.reserve((kCircleSegments + 1) * 2);
  for (int i = 0; i <= kCircleSegments; ++i) {
    Vec2 dir =
        Vec2::FromAngle(static_cast<float>(i) / kCircleSegments * kTwoPi);
    band.push_back(dir * outer);
    band.push_back(dir * inner);
  }
  Mat3 model = Mat3::Translate(center.x, center.y);
  DrawVerts(GL_TRIANGLE_STRIP, band, model, color);
}

void Renderer::DrawRect(Vec2 pos, Vec2 size, const Color& color) {
  static const std::vector<Vec2> kUnitQuad{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  Mat3 model = Mat3::Translate(pos.x, pos.y) * Mat3::Scale(size.x, size.y);
  DrawVerts(GL_TRIANGLE_FAN, kUnitQuad, model, color);
}

void Renderer::DrawTexturedQuad(Vec2 pos, Vec2 size, Vec2 uv_min, Vec2 uv_max,
                                const Texture& tex, const Color& color) {
  // Unit quad positioned by the model transform (mirrors DrawRect); each corner
  // carries the matching atlas UV so [uv_min, uv_max] maps onto the rect.
  const TexVertex quad[4] = {
      {{0, 0}, {uv_min.x, uv_min.y}},
      {{1, 0}, {uv_max.x, uv_min.y}},
      {{1, 1}, {uv_max.x, uv_max.y}},
      {{0, 1}, {uv_min.x, uv_max.y}},
  };
  Mat3 model = Mat3::Translate(pos.x, pos.y) * Mat3::Scale(size.x, size.y);
  textured_shader_->Use();
  textured_shader_->SetMat3("uProjection", screen_projection_);
  textured_shader_->SetMat3("uModel", model);
  textured_shader_->SetColor("uColor", color);
  tex.Bind(0);
  glBindVertexArray(tex_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Renderer::DrawBackground(const Texture& tex) {
  const TexVertex quad[4] = {
      {{0, 0}, {0, 0}}, {{1, 0}, {1, 0}}, {{1, 1}, {1, 1}}, {{0, 1}, {0, 1}}};
  Mat3 model = Mat3::Scale(overlay_extent_.x, overlay_extent_.y);
  image_shader_->Use();
  image_shader_->SetMat3("uProjection", screen_projection_);
  image_shader_->SetMat3("uModel", model);
  image_shader_->SetColor("uColor", {1.0f, 1.0f, 1.0f, 1.0f});
  tex.Bind(0);
  glBindVertexArray(tex_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Renderer::DrawVerts(unsigned int mode, const std::vector<Vec2>& pts,
                         const Mat3& model, const Color& color) {
  if (pts.empty()) {
    return;
  }
  shader_->Use();
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(pts.size() * sizeof(Vec2)), pts.data(),
               GL_DYNAMIC_DRAW);
  shader_->SetMat3("uModel", model);
  shader_->SetColor("uColor", color);
  glDrawArrays(mode, 0, static_cast<GLsizei>(pts.size()));
}

}  // namespace se::render
