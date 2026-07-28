# spacewar_sim -> se_core             headless model: World + entities
# Spacewar exe -> engine libs + sim   composition root + app/ presentation

add_library(spacewar_sim STATIC
  sim/ship.cpp
  sim/star.cpp
  sim/torpedo.cpp
  sim/world.cpp)
target_include_directories(spacewar_sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(spacewar_sim PUBLIC se_core)

add_executable(Spacewar
  ${CMAKE_SOURCE_DIR}/engine/main.cpp
  spacewar_game.cpp
  app/game_view.cpp
  app/hud.cpp
  app/effects.cpp
  app/app.cpp
  settings_io.cpp)
target_link_libraries(Spacewar
  PRIVATE se_engine spacewar_sim rapidjson)

# Absolute data paths baked in so the exe runs from any CWD and hot-reloads
# the in-tree files.
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
