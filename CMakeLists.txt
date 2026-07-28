cmake_minimum_required(VERSION 3.21)
project(SpacewarOOP C CXX)  # C is required by the vendored glad.c / GLFW sources

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "Spacewar targets Windows only.")
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR
    "Spacewar is x64 only; configure with an x64 generator/architecture "
    "(e.g. -A x64).")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(games/spacewar)
