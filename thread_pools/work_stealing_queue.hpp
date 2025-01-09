#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>
#include <random>

namespace work_stealing {
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

  std::shared_ptr<CircularArray> resize(int bottom, int top) {
    auto newTasks = std::make_shared<CircularArray>(logCapacity + 1);
    for (int i = top; i < bottom; ++i) {
      newTasks->put(i, get(i));
    }
    std::cout << "Resized!!" << std::endl;
    return newTasks;
  }
};

class UnboundedDEQueue {
private:
  static constexpr int LOG_CAPACITY = 4;
  std::atomic<int> bottom;
  std::atomic<int> top;
  std::shared_ptr<CircularArray> tasks;

public:
  UnboundedDEQueue()
      : bottom(0), top(0), tasks(std::make_shared<CircularArray>(LOG_CAPACITY)) {}

  bool isEmpty() {
    int localTop    = top.load(std::memory_order_acquire);
    int localBottom = bottom.load(std::memory_order_acquire);
    return (localBottom <= localTop);
  }

  void pushBottom(std::shared_ptr<std::function<void()>> r) {
    int oldBottom                               = bottom.load(std::memory_order_relaxed);
    int oldTop                                  = top.load(std::memory_order_acquire);
    std::shared_ptr<CircularArray> currentTasks = tasks;
    int size                                    = oldBottom - oldTop;
    if (size >= currentTasks->capacity() - 1) {
      currentTasks = currentTasks->resize(oldBottom, oldTop);
      tasks        = currentTasks;
    }
    tasks->put(oldBottom, r);
    bottom.store(oldBottom + 1, std::memory_order_release);
    std::cout << "Got " << oldBottom + 1 << std::endl;
  }

  std::shared_ptr<std::function<void()>> popTop() {
    int oldTop        = top.load(std::memory_order_acquire);
    int newTop        = oldTop + 1;
    int oldBottom     = bottom.load(std::memory_order_acquire);
    auto currentTasks = tasks;
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
    auto currentTasks = tasks;
    int oldBottom     = bottom.fetch_sub(1, std::memory_order_acq_rel) - 1;
    int oldTop        = top.load(std::memory_order_acquire);
    int size          = oldBottom - oldTop;
    if (size < 0) {
      bottom.store(oldTop, std::memory_order_release);
      return nullptr;
    }
    auto r = currentTasks->get(oldBottom);
    if (size > 0) {
      return r;
    }
    if (!top.compare_exchange_strong(oldTop, oldTop + 1, std::memory_order_acq_rel)) {
      r = nullptr;
    }
    bottom.store(oldTop + 1, std::memory_order_release);
    return r;
  }
};

class ThreadPool {
private:
  std::vector<std::thread> workers;
  std::vector<std::unique_ptr<UnboundedDEQueue>> queues;
  std::atomic<bool> stop;
  std::atomic<int> task_count;

  void workerThread(size_t index) {
    std::random_device rd;
    std::mt19937 gen(rd());

    while (!stop.load(std::memory_order_acquire)) {
      auto task = queues[index]->popBottom();
      //
      // if (!task) {
      //   // Try to steal tasks from other queues
      //   size_t numQueues = queues.size();
      //   std::uniform_int_distribution<size_t> dist(0, numQueues - 1);
      //
      //   for (size_t attempt = 0; attempt < numQueues; ++attempt) {
      //     size_t victim = dist(gen);
      //     if (victim == index) {
      //       continue;
      //     }
      //
      //     // task = queues[victim]->popTop();
      //     if (task) {
      //       break;
      //     }
      //   }
      // }
      //
      if (task) {
        (*task)();
        task_count.fetch_sub(1, std::memory_order_relaxed);

      } else {
        // std::cout << "Waitin on thread " << index << std::endl;
        std::this_thread::yield();
      }
    }
  }

public:
  ThreadPool(size_t numThreads)
      : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
      queues.emplace_back(std::make_unique<UnboundedDEQueue>());
    }

    for (size_t i = 0; i < numThreads; ++i) {
      workers.emplace_back(&ThreadPool::workerThread, this, i);
    }
  }

  ~ThreadPool() {
    stop.store(true, std::memory_order_release);
    for (auto &worker : workers) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  template <typename F, typename... Args>
  void Enqueue(F &&f, Args &&...args) {
    // TODO this is reasonable for multi consumer, but this should be at least random for each consumer
    size_t index = std::hash<std::thread::id>{}(std::this_thread::get_id()) % queues.size();

    auto task = std::make_shared<std::function<void()>>(
        [fn = std::forward<F>(f), ... capturedArgs = std::forward<Args>(args)]() mutable {
          fn(std::move(capturedArgs)...);
        }
    );

    queues[index]->pushBottom(task);
    task_count.fetch_add(1, std::memory_order_relaxed);
  }

  void Wait() {
    while (task_count.load(std::memory_order_relaxed) > 0) {
      std::this_thread::yield();
      std::cout << "Current task count: " << task_count.load(std::memory_order_relaxed) << std::endl;
    }
  }
};
} // namespace work_stealing
