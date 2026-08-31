#include "engine/render/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <stb_image.h>

namespace se::render {

namespace {

constexpr int kRedChannels = 1;
constexpr int kRgbaChannels = 4;

}  // namespace

std::unique_ptr<Texture> LoadTextureFile(const char* path) {
  int w = 0, h = 0, channels = 0;
  unsigned char* pixels = stbi_load(path, &w, &h, &channels, kRgbaChannels);
  if (pixels == nullptr) return nullptr;
  auto tex = std::make_unique<Texture>(w, h, kRgbaChannels, pixels);
  stbi_image_free(pixels);
  return tex->ok() ? std::move(tex) : nullptr;
}

Texture::Texture(int width, int height, int channels,
                 const unsigned char* pixels)
    : width_(width), height_(height) {
  if (width <= 0 || height <= 0 ||
      (channels != kRedChannels && channels != kRgbaChannels)) {
    return;  // id_ stays 0 -> ok() == false
  }
  GLenum format = (channels == kRedChannels) ? GL_RED : GL_RGBA;

  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0,
               format, GL_UNSIGNED_BYTE, pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture() { glDeleteTextures(1, &id_); }  // deleting 0 is a no-op

void Texture::Bind(unsigned int unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id_);
}

}  // namespace se::render
