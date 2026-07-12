#pragma once

#include "core/types.h"

namespace spacewar::render {

class GlContext {
 public:
  bool Load(GlProcLoader loader);

  void Resize(int width, int height);

  bool loaded() const { return loaded_; }

 private:
  bool loaded_ = false;
};

}  // namespace spacewar::render
