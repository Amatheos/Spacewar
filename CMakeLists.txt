cmake_minimum_required(VERSION 3.21)
project(SpacewarOOP C CXX)  # C is required by the vendored glad.c / GLFW sources

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "Spacewar-OOP targets Windows only.")
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR
    "Spacewar-OOP is x64 only; configure with an x64 generator/architecture "
    "(e.g. -A x64).")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(external)
add_subdirectory(src)
