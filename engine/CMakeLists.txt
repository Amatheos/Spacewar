# Engine layering (one-way), enforced by these target link rules
#
#   se_core     INTERFACE          types + math + input vocabulary. Header-only.
#   se_render   -> se_core, glad   GL context, Shader, primitive Renderer.
#   se_platform -> se_core, glfw   RAII Window + raw key facts. Never render.
#   se_audio    -> miniaudio       sound device + playback.
#
# Headers are reached as "engine/core/...", "engine/render/..." etc. via the
# repo root on the include path.

add_library(se_engine STATIC engine.cpp)
target_link_libraries(se_engine PUBLIC se_render se_platform se_audio)

add_library(se_core INTERFACE)
target_include_directories(se_core INTERFACE "${CMAKE_SOURCE_DIR}")

add_library(se_render STATIC
  render/gl_context.cpp
  render/shader.cpp
  render/texture.cpp
  render/font.cpp
  render/renderer.cpp)
target_link_libraries(se_render PUBLIC se_core PRIVATE glad stb)

add_library(se_platform STATIC
  platform/window.cpp)
target_link_libraries(se_platform PUBLIC se_core PRIVATE glfw)

# miniaudio_impl.cpp is the lone TU that compiles the miniaudio
# implementation. MSVC auto-links the OS audio libs via #pragma comment;
# MinGW/Unix need them named explicitly.
add_library(se_audio STATIC
  audio/engine.cpp
  audio/miniaudio_impl.cpp)
target_include_directories(se_audio PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(se_audio PRIVATE miniaudio)
if(MINGW)
  target_link_libraries(se_audio PRIVATE ole32 winmm)
elseif(UNIX AND NOT APPLE)
  target_link_libraries(se_audio PRIVATE pthread dl m)
endif()
