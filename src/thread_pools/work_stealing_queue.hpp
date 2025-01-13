/**
 * @file work_stealing_queue.h
 * @brief A work stealing queue
 * @date 2025-1-08
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <vector>
#include <thread>
#include <random>
#include <condition_variable>

namespace work_stealing {

extern volatile thread_local size_t me;

/**
 * @brief A circular array of tasks
 */
class CircularArray {
private:
  int logCapacity;
  std::vector<std::shared_ptr<std::function<void()>>> currentTasks;

public:
  /**
   * @brief Construct a new CircularArray object
   *
   * This constructor creates a new CircularArray with a capacity of 2^myLogCapacity.
   * @param myLogCapacity The logarithm of the capacity of this array.
   */
  explicit CircularArray(int myLogCapacity)
      : logCapacity(myLogCapacity), currentTasks(1 << myLogCapacity) {}

  /**
   * @brief Get the capacity of this array
   *
   * This is the power of 2 that was used to create this array.
   * @return The capacity of this array
   */
  int capacity() const {
    return 1 << logCapacity;
  }

  /**
   * @brief Get the task at the given index
   *
   * If the index is greater than the capacity of the array, it will be wrapped around to the beginning.
   * @param i The index of the task to get
   * @return The task at the given index
   */
  std::shared_ptr<std::function<void()>> get(int i) {
    return currentTasks[static_cast<size_t>(i % capacity())];
  }

  /**
   * @brief Put a task at the given index
   *
   * If the index is greater than the capacity of the array, it will be wrapped around to the beginning.
   * @param i The index of the task to set
   * @param task The task to set at that index
   */
  void put(int i, std::shared_ptr<std::function<void()>> task) {
    currentTasks[static_cast<size_t>(i % capacity())] = task;
  }

  /**
   * @brief Resize the circular array to have a capacity of 2^(logCapacity+1).
   *
   * This function will copy all the elements from index top to bottom to the new array.
   * The new array will be returned, and the old array will be deleted.
   * @param bottom The index of the bottom of the range to copy
   * @param top The index of the top of the range to copy
   * @return A pointer to the new resized array
   */
  CircularArray *resize(int bottom, int top) {
    auto newTasks = new CircularArray(logCapacity + 1);
    for (int i = top; i < bottom; ++i) {
      newTasks->put(i, get(i));
    }

    // This will dangle until there is a garbage collector in c++
    return newTasks;
  }
};

/**
 * @brief A work stealing queue
 *
 * This queue is unbounded but only supports a single consumer and a single producer from the same thread.
 */
class UnboundedDEQueue {
private:
  static constexpr int LOG_CAPACITY = 4;
  std::atomic<int> bottom;
  std::atomic<int> top;
  std::atomic<CircularArray *> tasks;

public:
  UnboundedDEQueue()
      : bottom(0), top(0), tasks(new CircularArray(LOG_CAPACITY)) {}

  ~UnboundedDEQueue() {
    delete tasks.load();
  }

  size_t size() {
    return (size_t)bottom.load(std::memory_order_relaxed) - (size_t)top.load(std::memory_order_relaxed);
  }

  size_t size() const {
    return (size_t)bottom.load(std::memory_order_relaxed) - (size_t)top.load(std::memory_order_relaxed);
  }

  bool isEmpty() {
    int localTop    = top.load(std::memory_order_acquire);
    int localBottom = bottom.load(std::memory_order_acquire);
    return (localBottom <= localTop);
  }

  /**
   * @brief Pushes a task to the bottom of the queue
   *
   * This method will grow the underlying array if the queue is full.
   * It will also delete the old underlying array if the queue is resized.
   *
   * @param r The task to be pushed
   */
  void pushBottom(std::shared_ptr<std::function<void()>> r) {
    int oldBottom               = bottom.load(std::memory_order_acquire);
    int oldTop                  = top.load(std::memory_order_acquire);
    CircularArray *currentTasks = tasks.load(std::memory_order_acquire);
    int size                    = oldBottom - oldTop;
    if (size >= currentTasks->capacity() - 1) {
      auto older   = currentTasks;
      currentTasks = currentTasks->resize(oldBottom, oldTop);
      tasks.store(currentTasks, std::memory_order_release);
      delete older;
    }
    tasks.load(std::memory_order_relaxed)->put(oldBottom, r);
    bottom.store(oldBottom + 1, std::memory_order_release);
  }

  /**
   * @brief Pops a task from the top of the queue
   *
   * Returns nullptr if the queue is empty, this method is used by stealing threads
   *
   * @return The task from the top of the queue
   */
  std::shared_ptr<std::function<void()>> popTop() {
    int oldTop        = top.load(std::memory_order_acquire);
    int newTop        = oldTop + 1;
    int oldBottom     = bottom.load(std::memory_order_acquire);
    auto currentTasks = tasks.load(std::memory_order_acquire);
    int size          = oldBottom - oldTop;
    if (size <= 0) {
      return nullptr;
    }
    auto r = currentTasks->get(oldTop);
    if (top.compare_exchange_strong(oldTop, newTop, std::memory_order_acq_rel)) {
      return r;
    }
    return nullptr;
  }

  /**
   * @brief Pops a task from the bottom of the queue
   *
   * This will try to pop a task from the bottom of the queue. If the queue is empty it will return nullptr.
   * If the queue is not empty, but the task has been stolen by another thread, it will also return nullptr.
   * @return The task from the bottom of the queue, or nullptr if the queue is empty or the task has been stolen
   */
  std::shared_ptr<std::function<void()>> popBottom() {
    // auto currentTasks = tasks.load(std::memory_order_seq_cst);
    --bottom;

    int oldTop = top.load(std::memory_order_acquire);
    int newTop = oldTop + 1;

    // We are out of elements, or someone just stole it
    int size = bottom.load(std::memory_order_acquire) - oldTop;
    if (size < 0) {
      bottom.store(oldTop, std::memory_order_release); // Setting the size to 0
      return nullptr;
    }

    // Only I can get the elements that are "overdue", if someone tries to steal
    auto r = tasks.load(std::memory_order_acquire)->get(bottom.load(std::memory_order_acquire));
    if (size > 0) {
      return r;
    }
    int b = oldTop;
    if (!top.compare_exchange_strong(oldTop, newTop, std::memory_order_acq_rel)) {
      r = nullptr;
    }
    bottom.store(b, std::memory_order_release);

    return r;
  }
};

/**
 * @brief A thread pool using work stealing queues
 */
class ThreadPool {
public:
  std::vector<std::unique_ptr<UnboundedDEQueue>> queues;
  std::atomic<int> task_count;

private:
  std::vector<std::thread> workers;
  std::atomic<bool> stop;
  std::uniform_int_distribution<size_t> dist;
  std::atomic<int> ready;
  std::atomic<int> waiting;
  std::condition_variable cv;
  std::mutex cv_mutex;

  /**
   * @brief The thread pool worker loop
   *
   * This function will be executed by each of the threads in the pool.
   * It will wait until all threads are ready and then start executing tasks.
   * The threads will steal tasks from each other until all tasks are done.
   * Once all tasks are done, the thread will wait until the next task is available.
   *
   * @param index the index of the thread in the pool
   */
  void workerThread(size_t index) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shared_ptr<std::function<void()>> task;
    me = index;

    auto &queue = *queues[index];

    // Wait for the first task

    // Sleep until explicitly notified, not get the task yet
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      ++waiting;
      cv.wait(lock, [&]() { return waiting.load(std::memory_order_relaxed) == static_cast<int>(workers.size()); });
    }

    // We can get the tasks now
    {
      while (!task) {
        task = queue.popTop();
        std::this_thread::yield();
      }
    }

    ++ready;
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.notify_all(); // Notify all threads about readiness
    }

    // Wait until all threads are ready
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.wait(lock, [&]() { return ready.load(std::memory_order_relaxed) == static_cast<int>(workers.size()); });
    }

    while (!stop.load()) {
      while (task) {
        (*task)();
        task_count.fetch_sub(1, std::memory_order_relaxed);
        if (task_count.load(std::memory_order_relaxed) == 0) {
          // Avoid a spin lock
          std::unique_lock<std::mutex> lock(cv_mutex);
          cv.notify_all();
        }
        task = queue.popBottom();
      }

      while (!task && !stop.load(std::memory_order_relaxed)) {
        std::this_thread::yield();

        task = queue.popBottom();
        if (!task) {
          size_t victim = dist(gen);
          task          = queues[victim]->popTop();
        }
      }
    }
  }

public:
  /**
   * @brief Construct a new ThreadPool object
   *
   * @param numThreads the number of threads to use in the pool
   *
   * This constructor will spawn the specified number of threads and start
   * the loop to wait for tasks.
   */
  ThreadPool(size_t numThreads)
      : task_count(0), stop(false), dist(0, numThreads - 1), ready(0), waiting(0) {
    for (size_t i = 0; i < numThreads; ++i) {
      queues.emplace_back(std::make_unique<UnboundedDEQueue>());
    }

    for (size_t i = 0; i < numThreads; ++i) {
      workers.emplace_back(&ThreadPool::workerThread, this, i);
    }
  }

  ~ThreadPool() {
    stop.store(true, std::memory_order_release);
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.notify_all();
    }

    for (auto &worker : workers) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  /**
   * @brief Submits a vector of tasks to the thread pool.
   *
   * @param fs the vector of tasks
   *
   * This function allows you to submit a vector of tasks to be executed by the thread pool.
   * The tasks will be executed by one of the available threads in the pool.
   *
   * The tasks are divided across the available threads, and the waiting threads are notified
   * after all tasks are submitted.
   */
  template <typename F>
  void Enqueue(std::vector<F> &&fs) {
    for (size_t i = 0; i < workers.size(); ++i) {
      queues[i]->pushBottom(std::make_shared<std::function<void()>>(fs[i]));
      task_count.fetch_add(1, std::memory_order_relaxed);
    }
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      // Wait until all threads are waitin on cv
      cv.wait(lock, [&]() { return waiting.load(std::memory_order_relaxed) == static_cast<int>(workers.size()); });

      cv.notify_all(); // Notify workers about new tasks
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
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&]() { return task_count.load(std::memory_order_relaxed) == 0; });
  }
};
} // namespace work_stealing
