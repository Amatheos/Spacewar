#include "engine/platform/window.h"

#include <GLFW/glfw3.h>

#include <utility>

namespace se::platform {
namespace {

constexpr int kGlVersionMajor = 3;
constexpr int kGlVersionMinor = 3;

constexpr int kGlfwKey[] = {
    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,
    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,
    GLFW_KEY_UP,
    GLFW_KEY_DOWN,
    GLFW_KEY_LEFT,
    GLFW_KEY_RIGHT,
    GLFW_KEY_SPACE,
    GLFW_KEY_ENTER,
    GLFW_KEY_ESCAPE,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_F1,
    GLFW_KEY_F2,
    GLFW_KEY_F3,
    GLFW_KEY_F4,
    GLFW_KEY_F5,
    GLFW_KEY_F6,
    GLFW_KEY_F7,
    GLFW_KEY_F8,
    GLFW_KEY_F9,
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
};
static_assert(std::size(kGlfwKey) == kKeyCount);

}  // namespace

GlfwLibrary::GlfwLibrary() {
  if (glfwInit() == GLFW_TRUE) {
    ok_ = true;
  }
}

GlfwLibrary::~GlfwLibrary() { glfwTerminate(); }

void Window::WindowDeleter::operator()(GLFWwindow* window) const {
  glfwDestroyWindow(window);
}

Window::Window(int width, int height, const char* title) {
  if (!lib_.ok()) return;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGlVersionMajor);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGlVersionMinor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  handle_.reset(glfwCreateWindow(width, height, title, nullptr, nullptr));
  if (!handle_) return;

  glfwMakeContextCurrent(handle_.get());
  glfwSwapInterval(1);  // vsync

  glfwSetWindowUserPointer(handle_.get(), this);
  glfwSetFramebufferSizeCallback(
      handle_.get(), [](GLFWwindow* w, int fb_w, int fb_h) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
        if (self && self->resize_cb_) self->resize_cb_(fb_w, fb_h);
      });
}

Window::~Window() = default;

bool Window::ShouldClose() const {
  return handle_ == nullptr || glfwWindowShouldClose(handle_.get()) != 0;
}

void Window::SwapBuffers() { glfwSwapBuffers(handle_.get()); }

void Window::PollEvents() { glfwPollEvents(); }

std::array<bool, kKeyCount> Window::KeysDown() const {
  std::array<bool, kKeyCount> res{};
  GLFWwindow* w = handle_.get();
  for (std::size_t i = 0; i < kKeyCount; ++i)
    res[i] = glfwGetKey(w, kGlfwKey[i]) == GLFW_PRESS;
  return res;
}

void Window::SetFullscreen(bool on) {
  if (!handle_ || on == fullscreen_) return;
  GLFWwindow* w = handle_.get();
  if (on) {
    glfwGetWindowPos(w, &win_x_, &win_y_);
    glfwGetWindowSize(w, &win_w_, &win_h_);
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    glfwSetWindowMonitor(w, mon, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  } else {
    glfwSetWindowMonitor(w, nullptr, win_x_, win_y_, win_w_, win_h_, 0);
  }
  glfwSwapInterval(1);  // some drivers drop vsync on a monitor switch
  fullscreen_ = on;
}

void Window::SetResizeCallback(std::function<void(int, int)> cb) {
  resize_cb_ = std::move(cb);
}

Window::Extent Window::FramebufferSize() const {
  int w = 0;
  int h = 0;
  glfwGetFramebufferSize(handle_.get(), &w, &h);
  return {w, h};
}

GlProcLoader Window::GlLoader() {
  return reinterpret_cast<GlProcLoader>(glfwGetProcAddress);
}

}  // namespace se::platform
