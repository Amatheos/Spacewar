# Third-party dependencies, fetched at configure time and version-pinned
# (ADR-005). First configure needs network; later configures reuse the
# populated build tree. Only glad stays in-tree: it is *generated* GL 3.3
# core loader code with no upstream to fetch.
include(FetchContent)

# --- GLFW 3.4 (builds via its own CMake) -------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# The three below are header-only: fetched for their sources, never built.
# Where upstream ships a CMakeLists.txt, SOURCE_SUBDIR points at a directory
# that has none, so FetchContent_MakeAvailable skips add_subdirectory and we
# expose plain INTERFACE targets instead. miniaudio's implementation TU must
# stay ours (src/audio/miniaudio_impl.cpp); building upstream's library too
# would duplicate every symbol.

# --- stb (single-header collection; pinned by commit, upstream has no tags) ---
FetchContent_Declare(stb
  URL https://github.com/nothings/stb/archive/31c1ad37456438565541f4919958214b6e762fb4.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# --- miniaudio 0.11.25 --------------------------------------------------------
FetchContent_Declare(miniaudio
  URL https://github.com/mackron/miniaudio/archive/refs/tags/0.11.25.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR no-cmake)

# --- RapidJSON (pinned by commit; the v1.1.0 release tag is a decade stale) ---
FetchContent_Declare(rapidjson
  URL https://github.com/Tencent/rapidjson/archive/24b5e7a8b27f42fa16b96fc70aade9106cf7102f.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR no-cmake)

FetchContent_MakeAvailable(glfw stb miniaudio rapidjson)

# SYSTEM: these headers are compiled into our TUs but are not ours to fix, so
# their warnings must not reach our -Werror. Marks them -isystem, not -I.
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE "${stb_SOURCE_DIR}")

add_library(miniaudio INTERFACE)
target_include_directories(miniaudio SYSTEM INTERFACE "${miniaudio_SOURCE_DIR}")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson SYSTEM INTERFACE "${rapidjson_SOURCE_DIR}/include")

# --- GLAD (in-tree generated OpenGL 3.3 Core loader) --------------------------
add_subdirectory(glad)
