#pragma once

#include <chrono>

class Timer {
public:
  Timer() {
    m_running = false;
  }

  void restart() {
    m_start   = std::chrono::high_resolution_clock::now();
    m_running = true;
  }

  void stop() {
    m_end     = std::chrono::high_resolution_clock::now();
    m_running = false;
  }

  double getElapsedSeconds() {
    return getElapsedMilliseconds() / 1000.0;
  }

  double getElapsedMilliseconds() {
    return getElapsedMicroseconds() / 1000.0;
  }

  double getElapsedMicroseconds() {
    return getElapsedNanoseconds() / 1000.0;
  }

  double getElapsedNanoseconds() {
    std::chrono::time_point<std::chrono::high_resolution_clock> end = m_running ? std::chrono::high_resolution_clock::now() : m_end;
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count());
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
  bool m_running;
};
