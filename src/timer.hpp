/**
 * @file timer.hpp
 * @brief A simple timer class.
 * @date 2025-1-08
 */

#pragma once

#include <chrono>

class Timer {
public:
  Timer() {
    m_running = false;
  }

  /**
   * @brief Restart the timer.
   *
   * This resets the start time to the current time and starts the timer.
   */
  void restart() {
    m_start   = std::chrono::high_resolution_clock::now();
    m_running = true;
  }

  /**
   * @brief Stop the timer.
   *
   * This records the current time as the stop time and stops the timer.
   *
   * @note Calling this function while the timer is already stopped has no effect.
   */
  void stop() {
    m_end     = std::chrono::high_resolution_clock::now();
    m_running = false;
  }

  /**
   * @brief Get the elapsed time in seconds.
   *
   * If the timer is currently running, the elapsed time is from the start time
   * to the current time. Otherwise, the elapsed time is from the start time to
   * the time at which the timer was stopped.
   *
   * @return The elapsed time in seconds.
   */
  double getElapsedSeconds() {
    return getElapsedMilliseconds() / 1000.0;
  }

  /**
   * @brief Get the elapsed time in milliseconds.
   *
   * If the timer is currently running, the elapsed time is from the start time
   * to the current time. Otherwise, the elapsed time is from the start time to
   * the time at which the timer was stopped.
   *
   * @return The elapsed time in milliseconds.
   */
  double getElapsedMilliseconds() {
    return getElapsedMicroseconds() / 1000.0;
  }


  /**
   * @brief Get the elapsed time in microseconds.
   *
   * If the timer is currently running, the elapsed time is from the start time
   * to the current time. Otherwise, the elapsed time is from the start time to
   * the time at which the timer was stopped.
   *
   * @return The elapsed time in microseconds.
   */
  double getElapsedMicroseconds() {
    return getElapsedNanoseconds() / 1000.0;
  }

  /**
   * @brief Get the elapsed time in nanoseconds.
   *
   * If the timer is currently running, the elapsed time is from the start time
   * to the current time. Otherwise, the elapsed time is from the start time to
   * the time at which the timer was stopped.
   *
   * @return The elapsed time in nanoseconds.
   */
  double getElapsedNanoseconds() {
    std::chrono::time_point<std::chrono::high_resolution_clock> end = m_running ? std::chrono::high_resolution_clock::now() : m_end;
    return static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start).count());
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
  bool m_running;
};
