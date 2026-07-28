#pragma once

#include <array>
#include <functional>
#include <memory>

#include "engine/core/input.h"
#include "engine/core/types.h"

struct GLFWwindow;

namespace se::platform {

class GlfwLibrary {
 public:
  GlfwLibrary();
  ~GlfwLibrary();
  GlfwLibrary(const GlfwLibrary&) = delete;
  GlfwLibrary& operator=(const GlfwLibrary&) = delete;
  bool ok() const { return ok_; }

 private:
  bool ok_ = false;
};

class Window {
 private:
  struct WindowDeleter {
    void operator()(GLFWwindow*) const;
  };

 public:
  struct Extent {
    int width = 0;
    int height = 0;
  };

  Window(int width, int height, const char* title);
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool ok() const { return handle_ != nullptr; }
  bool ShouldClose() const;
  void SwapBuffers();
  void PollEvents();

  std::array<bool, kKeyCount> KeysDown() const;

  void SetFullscreen(bool on);

  Extent FramebufferSize() const;

  void SetResizeCallback(std::function<void(int, int)> cb);

  static GlProcLoader GlLoader();

 private:
  GlfwLibrary lib_;
  std::unique_ptr<GLFWwindow, WindowDeleter> handle_;
  std::function<void(int, int)> resize_cb_;
  bool fullscreen_ = false;
  int win_x_ = 0, win_y_ = 0, win_w_ = 0, win_h_ = 0;  // saved windowed rect
};

}  // namespace se::platform
