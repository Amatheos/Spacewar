#pragma once

#include "engine/core/math.h"

namespace se::render {

class Shader {
 public:
  Shader(const char* vertex_src, const char* fragment_src);
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  // False if compilation or linking failed; the program is unusable and
  // Renderer::Init should bail.
  bool ok() const { return program_ != 0; }
  void SetMat3(const char* name, const Mat3& m) const;
  void SetColor(const char* name, const Color& color) const;
  void SetInt(const char* name, int value) const;
  void Use() const;

 private:
  unsigned int program_ = 0;
};

}  // namespace se::render
