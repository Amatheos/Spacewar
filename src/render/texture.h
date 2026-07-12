#pragma once

#include <memory>

namespace spacewar::render {

class Texture {
 public:
  // `pixels` is width*height*channels bytes, row-major, first row = top.
  // `channels` must be 1 (GL_RED) or 4 (GL_RGBA).
  Texture(int width, int height, int channels, const unsigned char* pixels);
  ~Texture();
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  bool ok() const { return id_ != 0; }
  int width() const { return width_; }
  int height() const { return height_; }

  void Bind(unsigned int unit = 0) const;

 private:
  unsigned int id_ = 0;
  int width_ = 0;
  int height_ = 0;
};

std::unique_ptr<Texture> LoadTextureFile(const char* path);

}  // namespace spacewar::render
