cmake_minimum_required(VERSION 3.21)
project(SpacewarOOP C CXX)  # C is required by the vendored glad.c / GLFW sources

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR "Spacewar is 64-bit only.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Warning policy. Linked PRIVATE by our own targets only -- never by fetched
# dependencies, which are not ours to fix and would fail under -Werror.
option(SE_WARNINGS_AS_ERRORS "Fail the build on any compiler warning" ON)

add_library(se_warnings INTERFACE)
target_compile_options(se_warnings INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wnon-virtual-dtor
  -Woverloaded-virtual
  -Wnull-dereference
  -Wimplicit-fallthrough
  -Wformat=2
  $<$<BOOL:${SE_WARNINGS_AS_ERRORS}>:-Werror>)

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(games/spacewar)
add_subdirectory(games/boids)
