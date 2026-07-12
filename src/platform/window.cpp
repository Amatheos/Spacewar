#include "platform/window.h"

#include <GLFW/glfw3.h>

#include <utility>

namespace spacewar::platform {

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
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

MenuInput Window::PollMenu() const {
  auto down = [w = handle_.get()](int key) {
    return glfwGetKey(w, key) == GLFW_PRESS;
  };
  MenuInput m;
  m.up = down(GLFW_KEY_UP) || down(GLFW_KEY_W);
  m.down = down(GLFW_KEY_DOWN) || down(GLFW_KEY_S);
  m.left = down(GLFW_KEY_LEFT) || down(GLFW_KEY_A);
  m.right = down(GLFW_KEY_RIGHT) || down(GLFW_KEY_D);
  m.select = down(GLFW_KEY_ENTER) || down(GLFW_KEY_SPACE);
  m.back = down(GLFW_KEY_ESCAPE) || down(GLFW_KEY_BACKSPACE);
  m.debug = down(GLFW_KEY_F1);
  return m;
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

GameInput Window::PollInput() const {
  auto down = [w = handle_.get()](int key) {
    return glfwGetKey(w, key) == GLFW_PRESS;
  };

  GameInput in;
  ShipInput& p1 = in.players[0];  // needle
  p1.rotate_left = down(GLFW_KEY_A);
  p1.rotate_right = down(GLFW_KEY_D);
  p1.thrust = down(GLFW_KEY_W);
  p1.fire = down(GLFW_KEY_LEFT_SHIFT);
  p1.hyperspace = down(GLFW_KEY_LEFT_CONTROL);

  ShipInput& p2 = in.players[1];  // wedge
  p2.rotate_left = down(GLFW_KEY_LEFT);
  p2.rotate_right = down(GLFW_KEY_RIGHT);
  p2.thrust = down(GLFW_KEY_UP);
  p2.fire = down(GLFW_KEY_RIGHT_SHIFT);
  p2.hyperspace = down(GLFW_KEY_RIGHT_CONTROL);
  return in;
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

}  // namespace spacewar::platform
