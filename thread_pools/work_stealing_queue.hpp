#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <random>
#include <condition_variable>

namespace work_stealing {

volatile thread_local size_t me;

class CircularArray {
private:
  int logCapacity;
  std::vector<std::shared_ptr<std::function<void()>>> currentTasks;

public:
  explicit CircularArray(int myLogCapacity)
      : logCapacity(myLogCapacity), currentTasks(1 << myLogCapacity) {}

  int capacity() const {
    return 1 << logCapacity;
  }

  std::shared_ptr<std::function<void()>> get(int i) {
    return currentTasks[static_cast<size_t>(i % capacity())];
  }

  void put(int i, std::shared_ptr<std::function<void()>> task) {
    currentTasks[static_cast<size_t>(i % capacity())] = task;
  }

  CircularArray *resize(int bottom, int top) {
    auto newTasks = new CircularArray(logCapacity + 1);
    for (int i = top; i < bottom; ++i) {
      newTasks->put(i, get(i));
    }

    // This will dangle until there is a garbage collector in c++
    return newTasks;
  }
};

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

class ThreadPool {
public:
  std::vector<std::unique_ptr<UnboundedDEQueue>> queues;
  std::atomic<int> task_count;

private:
  std::vector<std::thread> workers;
  std::atomic<bool> stop;
  std::uniform_int_distribution<size_t> dist;
  std::atomic<int> ready;
  std::condition_variable cv;
  std::mutex cv_mutex;

  void workerThread(size_t index) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shared_ptr<std::function<void()>> task;
    me = index;

    // Wait for the first task

    // Sleep until explicitly notified, not get the task yet
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.wait(lock);
    }

    // We can get the tasks now
    {
      while (!task) {
        task = queues[index]->popTop();
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
        if (task_count == 0) {
          std::unique_lock<std::mutex> lock(cv_mutex);
          cv.notify_all();
        }
        task = queues[index]->popBottom();
      }

      while (!task && !stop.load()) {
        std::this_thread::yield();
        size_t victim = dist(gen);
        task          = queues[victim]->popTop();
      }
    }
  }

public:
  ThreadPool(size_t numThreads)
      : task_count(0), stop(false), dist(0, numThreads - 1), ready(0) {
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

  template <typename F>
  void Enqueue(std::vector<F> &&fs) {
    for (size_t i = 0; i < workers.size(); ++i) {
      queues[i]->pushBottom(std::make_shared<std::function<void()>>(fs[i]));
      task_count.fetch_add(1, std::memory_order_relaxed);
    }
    {
      std::unique_lock<std::mutex> lock(cv_mutex);
      cv.notify_all(); // Notify workers about new tasks
    }
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&]() { return task_count.load(std::memory_order_relaxed) == 0; });
  }
};
} // namespace work_stealing
