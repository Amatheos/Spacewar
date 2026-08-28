# Engine layering (one-way), enforced by these target link rules
#
#   se_core     INTERFACE          types + math + input vocabulary. Header-only.
#   se_render   -> se_core, glad   GL context, Shader, primitive Renderer.
#   se_platform -> se_core, glfw   RAII Window + raw key facts. Never render.
#   se_profiler -> se_core         scoped timers. Anything may depend on it.
#   se_audio    -> miniaudio       sound device + playback.
#
# Headers are reached as "engine/core/...", "engine/render/..." etc. via the
# repo root on the include path.

add_library(se_engine STATIC engine.cpp)
target_link_libraries(se_engine PUBLIC se_render se_platform se_audio se_profiler PRIVATE se_warnings)

add_library(se_core INTERFACE)
target_include_directories(se_core INTERFACE "${CMAKE_SOURCE_DIR}")

add_library(se_render STATIC
  render/gl_context.cpp
  render/shader.cpp
  render/texture.cpp
  render/font.cpp
  render/renderer.cpp
  render/camera.cpp)
target_link_libraries(se_render PUBLIC se_core PRIVATE glad stb se_warnings)

add_library(se_platform STATIC
  platform/window.cpp)
target_link_libraries(se_platform PUBLIC se_core PRIVATE glfw se_warnings)

add_library(se_profiler INTERFACE)
target_link_libraries(se_profiler INTERFACE se_core)

# miniaudio_impl.cpp is the lone TU that compiles the miniaudio
# implementation; it needs the OS audio libs named explicitly.
add_library(se_audio STATIC
  audio/engine.cpp
  audio/miniaudio_impl.cpp)
target_include_directories(se_audio PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(se_audio PRIVATE miniaudio pthread dl m)
