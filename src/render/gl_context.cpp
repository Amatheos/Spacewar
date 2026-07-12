#include "render/gl_context.h"

#include <glad/glad.h>

namespace spacewar::render {

bool GlContext::Load(GlProcLoader loader) {
  loaded_ = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(loader)) != 0;
  return loaded_;
}

void GlContext::Resize(int width, int height) {
  glViewport(0, 0, width, height);
}

}  // namespace spacewar::render
