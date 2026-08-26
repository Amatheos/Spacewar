add_executable(Boids
  ${CMAKE_SOURCE_DIR}/engine/main.cpp
  boids_game.cpp
  sim/flock.cpp
  app/flock_view.cpp)

target_include_directories(Boids PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(Boids PRIVATE se_engine se_warnings)
