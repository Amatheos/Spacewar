#pragma once

#include "engine/core/types.h"

namespace se::render {

class GlContext {
 public:
  bool Load(GlProcLoader loader);

  void Resize(int width, int height);

  bool loaded() const { return loaded_; }

 private:
  bool loaded_ = false;
};

}  // namespace se::render
