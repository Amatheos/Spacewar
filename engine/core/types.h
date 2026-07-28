#pragma once

namespace se {

// Function that resolves an OpenGL function pointer by name (the shape of
// glfwGetProcAddress). The platform layer hands one of these to the renderer so
// neither side has to share a GLFW or OpenGL header -- it is the only "GL-ish"
// type that crosses the platform/render boundary, keeping the glad-including and
// glfw-including modules cleanly separated.
using GlProcLoader = void* (*)(const char*);

}  // namespace se
