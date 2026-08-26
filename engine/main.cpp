#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "engine/engine.h"
#include "engine/game.h"
#include "engine/profiler/profiler.h"

namespace {

int Usage(const char* exe, const char* problem, const char* arg) {
  std::fprintf(stderr, "%s: %s: %s\n", exe, problem, arg);
  std::fprintf(stderr, "usage: %s [-headless] [-ticks N] [-trace PATH]\n", exe);
  return 2;
}

}  // namespace

int main(int argc, char** argv) {
  bool headless = false;
  int num_ticks = 10'000;
  const char* trace_path = "profile.json";

  for (int i = 1; i < argc; ++i) {
    const std::string_view arg(argv[i]);
    if (arg == "-headless") {
      headless = true;
    } else if (arg == "-trace") {
      if (i + 1 >= argc) return Usage(argv[0], "missing value for", argv[i]);
      trace_path = argv[++i];
    } else if (arg == "-ticks") {
      if (i + 1 >= argc) return Usage(argv[0], "missing value for", argv[i]);
      const int num = std::atoi(argv[++i]);
      if (num <= 0)
        return Usage(argv[0], "expected a positive integer after -ticks",
                     argv[i]);
      num_ticks = num;
    } else {
      return Usage(
          argv[0],
          arg.starts_with("-") ? "unknown option" : "unexpected argument",
          argv[i]);
    }
  }

  se::profiler::Init();
  se::profiler::Session s(trace_path);
  se::Engine e;
  return e.Run({headless, num_ticks}, {}, se::CreateGame());
}
