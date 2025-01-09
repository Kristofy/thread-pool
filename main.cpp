#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ostream>

// Unity build
#include "collatz.cpp"
#include "colors.hpp"
#include "thread_pools/locking_threadpool.hpp"
// #include "thread_pools/compare_exchange_threadpool.hpp"
#include "thread_pools/work_stealing_queue.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "statistics.hpp"

/**
 * @brief An iterative method of the collatz conjectures famous sequence.
 *
 * @param n The number to start the sequence from.
 * @return The number of steps it took to reach 1.
 */
inline int16_t collatz(uint64_t n) {
  // Proof by wikipedia: less than 10^12 is 989345275647, which has 1348 steps
  // Meaning that for the maximum uint32 -> 2^32-1 which is < 10^12 -> meaning steps can be integer
  int16_t steps = 0;
  while (n != 1) {
    if (n % 2 == 0) {
      n /= 2;
    } else {
      n = 3 * n + 1;
    }
    steps++;
  }
  return steps;
}

/**
 * @brief Times the collatz conjecture with a function
 *
 * @tparam Fn The function type
 * @param fn the function that retuns the collatz sequence up until n
 * @param n The number to calculate the collatz sequence until
 * @param name The name of the function
 * @param checksum The checksum of the collatz sequence
 * @return
 */
template <typename Fn>
double time_collatz_with_function(Fn fn, uint64_t n, std::string name, uint64_t collatz_checksum) {
  Timer timer;
  timer.restart();
  auto answer = fn(n);
  timer.stop();

  if (checksum(answer) != collatz_checksum) {
    std::cout << "Checksum failed for: " << name << std::endl;

    std::cout << "Expected: " << collatz_checksum << std::endl;
    std::cout << "Got: " << checksum(answer) << std::endl;
    std::abort();
  }

  auto time = timer.getElapsedMilliseconds();
  return time;
}

std::vector<int16_t> naive(uint64_t n) {
  std::vector<int16_t> collatz_values(n);
  for (uint64_t i = 1; i <= n; i++) {
    collatz_values[i - 1] = collatz(i);
  }
  return collatz_values;
}

std::vector<int16_t> locking_threadpool(uint64_t n) {
  std::vector<int16_t> collatz_values(n);
  locking::ThreadPool tp(std::thread::hardware_concurrency());

  constexpr int res = 100;

  auto action = +[](uint64_t from, uint64_t to, int16_t *out) {
    for (uint64_t i = from; i < to; i++) {
      *out = collatz(i);
      ++out;
    }
  };

  for (uint64_t i = 1; i <= n; i += res) {
    tp.Enqueue(action, i, std::min(i + res, n + 1), collatz_values.data() + i - 1);
  }

  tp.Wait();

  return collatz_values;
}

// std::vector<int16_t> compare_exchange_threadpool(uint64_t n) {
//   std::vector<int16_t> collatz_values(n);
//   compare_exchange::ThreadPool tp(std::thread::hardware_concurrency());
//
//   constexpr int res = 1;
//
//   auto action = +[](uint64_t from, uint64_t to, int16_t *out) {
//     for (uint64_t i = from; i < to; i++) {
//       *out = collatz(i);
//       ++out;
//     }
//   };
//
//   for (uint64_t i = 1; i <= n; i += res) {
//     tp.Enqueue(action, i, std::min(i + res, n + 1), collatz_values.data() + i - 1);
//   }
//
//   tp.Wait();
//
//   // Check the values
//   std::cout << "dbg: ";
//   for (int i = 0; i < 100; i++) {
//     std::cout << collatz_values[i] << " ";
//   }
//   std::flush(std::cout);
//
//   return collatz_values;
// }

std::vector<int16_t> work_stealing_threadpool(uint64_t n) {
  std::vector<int16_t> collatz_values(n);
  work_stealing::ThreadPool tp(std::thread::hardware_concurrency() - 2);

  constexpr int res = 16;

  auto action = +[](uint64_t from, uint64_t to, int16_t *out) {
    for (uint64_t i = from; i < to; i++) {
      *out = collatz(i);
      ++out;
    }
  };

  for (uint64_t i = 1; i <= n; i += res) {
    tp.Enqueue(action, i, std::min(i + res, n + 1), collatz_values.data() + i - 1);
  }

  tp.Wait();

  return collatz_values;
}

int main() {

  // const int n_max                 = 1e5;
  const int n_max                 = 20000;
  const int k_max                 = 10;
  const int segments_max          = 10;
  const uint64_t collatz_checksum = get_collatz_checksum_until(n_max, true, true);

  std::vector<statistics> stats;
  std::vector<std::tuple<std::string, std::function<std::vector<int16_t>(uint64_t)>>> functions = {
      {"Naive     ", naive},
      {"Locking TP", locking_threadpool},
      // {"CAS TP", compare_exchange_threadpool},
      {"Ws CAS TP", work_stealing_threadpool},
  };

  std::vector<double> times;
  for (const auto &[name, function] : functions) {
    std::cout << colorTerminal(TerminalColor::TC_BRIGHT_WHITE, true) << name << ": " << resetTerminal() << "\t|" << std::string(segments_max, '-') << "|";
    std::flush(std::cout);

    for (int k = 0; k < k_max; k++) {
      auto time = time_collatz_with_function(function, n_max, name, collatz_checksum);
      times.push_back(time);
      std::cout << '\r' << colorTerminal(TerminalColor::TC_BRIGHT_WHITE, true) << name << ": " << resetTerminal() << "\t|";
      for (int i = 0; i < ((k + 1) * segments_max + k_max - 1) / k_max; i++) {
        std::cout << "█";
      }
      std::cout << std::string(static_cast<size_t>(segments_max - ((k + 1) * segments_max + k_max - 1) / k_max), '-') << "|";
      std::flush(std::cout);
    }

    // Delete the current line
    std::cout << '\r' << std::string(100, ' ') << '\r';
    statistics::print_statistics({statistics::from_vector(times, name)});

    stats.push_back(statistics::from_vector(times, name));
  }

  std::cout << std::endl;
  std::cout << "Results: " << std::endl;

  statistics::print_statistics(stats);

  // Baseline

  return 0;
}
