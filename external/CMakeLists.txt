# Vendored third-party dependencies. Both are built from in-tree sources so the
# project has no external download or install step.

# --- GLFW (vendored source tree) ------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# --- GLAD (vendored generated OpenGL 3.3 Core loader) ---------------------
add_subdirectory(glad)

# --- RapidJSON (vendored, header-only) ------------------------------------
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include")

# --- stb_truetype (vendored, single-header) -------------------------------
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# --- miniaudio (vendored, single-header) ----------------------------------
# Its implementation is large and slow to compile, so
# MINIAUDIO_IMPLEMENTATION lives in its own dedicated TU
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio")
