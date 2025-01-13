#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>

#include "thread_pools/locking_threadpool.hpp"
#include "thread_pools/work_stealing_queue.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "collatz.h"
#include "statistics.hpp"

// Unity build
#include "collatz.cpp"
#include "thread_pools/work_stealing_queue.cpp"

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

template <int res, size_t cores>
std::vector<int16_t> locking_threadpool(uint64_t n) {
  std::vector<int16_t> collatz_values(n);
  locking::ThreadPool tp(cores);

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

template <int res, size_t threads>
std::vector<int16_t> work_stealing_threadpool(uint64_t n) {
  work_stealing::ThreadPool tp(threads);

  std::vector<int16_t> collatz_values(n);
  std::vector<std::function<void()>> tasks;

  auto data = collatz_values.data();
  for (uint64_t i = 0; i < threads; i++) {

    tasks.push_back([&tp, n, data, i]() {
      for (size_t j = 1; j <= n; j += res * threads) {

        size_t from = j + i * res;
        size_t to   = std::min(j + res * (i + 1), n + 1);

        tp.queues[work_stealing::me]->pushBottom(std::make_shared<std::function<void()>>([from, to, data]() {
          for (uint64_t l = from; l < to; l++) {
            data[l - 1] = collatz(l);
          }
        }));
        tp.task_count.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  tp.Enqueue(std::move(tasks));

  tp.Wait();

  return collatz_values;
}

int main() {

  // const int n_max                 = 1e5;
  const int n_max                 = 1000000;
  const int k_max                 = 10;
  const int segments_max          = 10;
  const uint64_t collatz_checksum = get_collatz_checksum_until(n_max, true, true);

  std::vector<statistics> stats;
  std::vector<std::tuple<std::string, std::function<std::vector<int16_t>(uint64_t)>>> functions = {

      {"Naive     ", naive},

      {"LockingTP 100b / 4t", locking_threadpool<n_max / 100, 4>},
      {"LockingTP 1000b / 4t", locking_threadpool<n_max / 1000, 4>},
      {"LockingTP 10000b / 4t", locking_threadpool<n_max / 10000, 4>},
      {"Ws_CAS_TP 100b / 4t", work_stealing_threadpool<n_max / 100, 4>},
      {"Ws_CAS_TP 1000b / 4t", work_stealing_threadpool<n_max / 1000, 4>},
      {"Ws_CAS_TP 10000b / 4t", work_stealing_threadpool<n_max / 10000, 4>},

      {"LockingTP 100b / 6t", locking_threadpool<n_max / 100, 6>},
      {"LockingTP 1000b / 6t", locking_threadpool<n_max / 1000, 6>},
      {"LockingTP 10000b / 6t", locking_threadpool<n_max / 10000, 6>},
      {"Ws_CAS_TP 100b / 6t", work_stealing_threadpool<n_max / 100, 6>},
      {"Ws_CAS_TP 1000b / 6t", work_stealing_threadpool<n_max / 1000, 6>},
      {"Ws_CAS_TP 10000b / 6t", work_stealing_threadpool<n_max / 10000, 6>},

      {"LockingTP 100b / 8t", locking_threadpool<n_max / 100, 8>},
      {"LockingTP 1000b / 8t", locking_threadpool<n_max / 1000, 8>},
      {"LockingTP 10000b / 8t", locking_threadpool<n_max / 10000, 8>},
      {"Ws_CAS_TP 100b / 8t", work_stealing_threadpool<n_max / 100, 8>},
      {"Ws_CAS_TP 1000b / 8t", work_stealing_threadpool<n_max / 1000, 8>},
      {"Ws_CAS_TP 10000b / 8t", work_stealing_threadpool<n_max / 10000, 8>},

      {"LockingTP 100b / 16t", locking_threadpool<n_max / 100, 16>},
      {"LockingTP 1000b / 16t", locking_threadpool<n_max / 1000, 16>},
      {"LockingTP 10000b / 16t", locking_threadpool<n_max / 10000, 16>},
      {"Ws_CAS_TP 100b / 16t", work_stealing_threadpool<n_max / 100, 16>},
      {"Ws_CAS_TP 1000b / 16t", work_stealing_threadpool<n_max / 1000, 16>},
      {"Ws_CAS_TP 10000b / 16t", work_stealing_threadpool<n_max / 10000, 16>},

      {"LockingTP 100b / 32t", locking_threadpool<n_max / 100, 32>},
      {"LockingTP 1000b / 32t", locking_threadpool<n_max / 1000, 32>},
      {"LockingTP 10000b / 32t", locking_threadpool<n_max / 10000, 32>},
      {"Ws_CAS_TP 100b / 32t", work_stealing_threadpool<n_max / 100, 32>},
      {"Ws_CAS_TP 1000b / 32t", work_stealing_threadpool<n_max / 1000, 32>},
      {"Ws_CAS_TP 10000b / 32t", work_stealing_threadpool<n_max / 10000, 32>},

      {"LockingTP 100b / 64t", locking_threadpool<n_max / 100, 64>},
      {"LockingTP 1000b / 64t", locking_threadpool<n_max / 1000, 64>},
      {"LockingTP 10000b / 64t", locking_threadpool<n_max / 10000, 64>},
      {"Ws_CAS_TP 100b / 64t", work_stealing_threadpool<n_max / 100, 64>},
      {"Ws_CAS_TP 1000b / 64t", work_stealing_threadpool<n_max / 1000, 64>},
      {"Ws_CAS_TP 10000b / 64t", work_stealing_threadpool<n_max / 10000, 64>},
  };

  for (const auto &[name, function] : functions) {
    std::vector<double> times;
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
