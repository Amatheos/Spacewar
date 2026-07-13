# Layering (one-way), enforced structurally by these target link rules
# 
#   core     INTERFACE          types + math. Header-only.
#   render   -> core, glad      GL context, Shader, primitive Renderer. Game-agnostic.
#   platform -> core, glfw      RAII Window + neutral input POD. Never render/game.
#   game     -> core            pure-sim entities + World. Render-free, headless.
#   audio    -> miniaudio       sound device + playback. Game-agnostic.
#   Spacewar -> all             the executable: composition root (main.cpp) + the
#                               src/app presentation bridges (GameView, Hud,
#                               Effects, player_palette) + settings IO. Sees
#                               game+render and turns sim state into neutral
#                               render frames.
#
# Every module header is reached as "core/...", "render/...", etc. via the src
# dir on the include path.

# --- core: header-only types + math + the neutral input/GL contract PODs -----
add_library(spacewar_core INTERFACE)
target_include_directories(spacewar_core INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

# --- render: GL drawing. Links glad privately so consumers never see GL ------
add_library(spacewar_render STATIC
  render/gl_context.cpp
  render/shader.cpp
  render/texture.cpp
  render/font.cpp
  render/renderer.cpp)
target_include_directories(spacewar_render PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(spacewar_render PUBLIC spacewar_core PRIVATE glad stb)

# --- platform: RAII window + input polling. Links glfw privately -------------
add_library(spacewar_platform STATIC
  platform/window.cpp)
target_include_directories(spacewar_platform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(spacewar_platform PUBLIC spacewar_core PRIVATE glfw)

# --- game: pure-sim entities + World. Render-free and headless (core only) ---
add_library(spacewar_game STATIC
  game/ship.cpp
  game/star.cpp
  game/torpedo.cpp
  game/world.cpp)
target_include_directories(spacewar_game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(spacewar_game PUBLIC spacewar_core)

# --- audio: sound device + playback. Links miniaudio privately ---------------
# miniaudio_impl.cpp is the lone TU that compiles the vendored implementation.
# MSVC auto-links the OS audio libs via #pragma comment; MinGW/Unix need them
# named explicitly.
add_library(spacewar_audio STATIC
  audio/engine.cpp
  audio/miniaudio_impl.cpp)
target_include_directories(spacewar_audio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(spacewar_audio PRIVATE miniaudio)
if(MINGW)
  target_link_libraries(spacewar_audio PRIVATE ole32 winmm)
elseif(UNIX AND NOT APPLE)
  target_link_libraries(spacewar_audio PRIVATE pthread dl m)
endif()

# --- Spacewar: the game executable (composition root + presentation view) ----
add_executable(Spacewar
  main.cpp
  app/game_view.cpp
  app/hud.cpp
  app/effects.cpp
  app/app.cpp
  settings_io.cpp)
target_link_libraries(Spacewar
  PRIVATE spacewar_platform spacewar_render spacewar_game spacewar_audio
          spacewar_core rapidjson)

# Bake an absolute path to the in-tree gameplay settings so the exe finds them
# regardless of the launcher's working directory, while still hot-reloading
# the real source file.
target_compile_definitions(Spacewar PRIVATE
  SPACEWAR_SETTINGS_PATH="${CMAKE_SOURCE_DIR}/data/gameplay_settings.json"
  SPACEWAR_OPTIONS_PATH="${CMAKE_SOURCE_DIR}/data/options.json"
  SPACEWAR_FONT_PATH="${CMAKE_SOURCE_DIR}/data/home-video.regular.ttf"
  SPACEWAR_DATA_DIR="${CMAKE_SOURCE_DIR}/data/")

if(MSVC)
  target_link_options(Spacewar PRIVATE
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
elseif(WIN32)
  target_link_options(Spacewar PRIVATE
    $<$<CONFIG:Release>:-mwindows>)
endif()
