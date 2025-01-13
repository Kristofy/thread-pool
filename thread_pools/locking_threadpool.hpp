/**
 * @file locking_threadpool.hpp
 * @brief A thread pool with a blocking queue
 * @date 2025-1-08
 */

#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace locking {
/**
 * @brief A thread pool with a blocking queue
 */
class ThreadPool {
public:
  /**
   * @brief Construct a new ThreadPool object
   * 
   * @param _num_threads the number of threads to use in the pool
   * 
   * This constructor will spawn the specified number of threads and start
   * the loop to wait for tasks. 
   */
  explicit ThreadPool(size_t _num_threads)
      : num_threads(_num_threads), task_counter(0), stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
      workers.emplace_back([this]() {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->queue_condition.wait(
                lock, [this]() { return this->stop || !this->tasks.empty(); }
            );
            if (this->stop && this->tasks.empty()) {
              return;
            }
            task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          task();
          {
            std::unique_lock<std::mutex> lock(this->task_counter_mutex);
            this->task_counter--;
            this->counter_condition.notify_one();
          }
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    queue_condition.notify_all();
    for (std::thread &worker : workers) {
      worker.join();
    }
  }

  /**
   * @brief Submits a task to the thread pool.
   *
   * This function allows you to submit a task with the given arguments to be executed by the thread pool.
   * The task will be executed by one of the available threads in the pool.
   *
   * @param func The function to be executed by the thread pool.
   * @param args The arguments to be passed to the task.
   */
  template <class F, class... Args>
  void Enqueue(F &&func, Args &&...args) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      tasks.emplace([func, args...]() { func(args...); });
      queue_condition.notify_one();
    }

    {
      std::unique_lock<std::mutex> lock(task_counter_mutex);
      this->task_counter++;
    }
  }

  /**
   * @brief Blocks the calling thread until all tasks have been completed.
   *
   * This function will block the calling thread until all tasks that have been submitted to the thread pool
   * have been completed. This is useful for cleanup or for waiting until all tasks have finished before
   * shutting down the thread pool.
   */
  void Wait() {
    std::unique_lock<std::mutex> lock(task_counter_mutex);
    counter_condition.wait(lock, [this]() { return this->task_counter == 0; });
  }

  /**
   * @brief Returns the number of threads in the thread pool.
   *
   * @return The number of threads in the thread pool.
   */
  size_t GetThreadCount() const { return num_threads; }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::mutex task_counter_mutex;
  std::condition_variable queue_condition;
  std::condition_variable counter_condition;
  const size_t num_threads;
  size_t task_counter;
  bool stop;
};

} // namespace locking
