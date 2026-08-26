#pragma once

#include <chrono>
#include <cstdio>
#include <fstream>

namespace se::profiler {

using Clock = std::chrono::steady_clock;

struct ProfileResult {
  const char* name;
  Clock::time_point start;
  Clock::duration duration;
};

inline std::ofstream output_stream;
inline int profile_count = 0;

inline void Init() {
#ifdef NDEBUG
  std::printf("[profiler] optimized build\n");
#else
  std::printf("[profiler] UNOPTIMIZED build - timings are not meaningful\n");
#endif
}

class Session {
 public:
  explicit Session(const char* filepath) {
    if (output_stream.is_open()) {
      std::fprintf(stderr, "[profiler] session already active, ignoring '%s'\n",
                   filepath);
      return;
    }
    profile_count = 0;
    output_stream.open(filepath);
    output_stream << "{\"otherData\": {},\"traceEvents\":[";
  }

  ~Session() {
    output_stream << "]}";
    output_stream.close();
  }

  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
};

inline long long Micros(Clock::duration d) {
  return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

inline void Report(const ProfileResult& res) {
  if (profile_count++ > 0) output_stream << ",";

  output_stream << "{";
  output_stream << "\"cat\":\"function\",";
  output_stream << "\"dur\":" << Micros(res.duration) << ',';
  output_stream << "\"name\":\"" << res.name << "\",";
  output_stream << "\"ph\":\"X\",";
  output_stream << "\"pid\":0,";
  output_stream << "\"tid\":0,";
  output_stream << "\"ts\":" << Micros(res.start.time_since_epoch());
  output_stream << "}";

  output_stream.flush();
}

class ScopedTimer {
 private:
  class Timer {
   public:
    Timer() { Reset(); }

    void Reset() { start_ = Clock::now(); }

    Clock::time_point Start() const { return start_; }

    Clock::duration Elapsed() const { return Clock::now() - start_; }

    float ElapsedMillis() const {
      return std::chrono::duration<float, std::milli>(Clock::now() - start_)
          .count();
    }
    float ElapsedSeconds() const { return ElapsedMillis() * 0.001f; }

   private:
    Clock::time_point start_;
  };

 public:
  explicit ScopedTimer(const char* name) : name_(name) {}
  ~ScopedTimer() { Report({name_, timer_.Start(), timer_.Elapsed()}); }

  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;

 private:
  const char* name_;
  Timer timer_;
};

}  // namespace se::profiler
