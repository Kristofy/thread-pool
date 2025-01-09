#include <iostream>
#include <memory>
#include <functional>
#include "thread_pools/work_stealing_queue.hpp" // Assume the class is in this header file

namespace work_stealing {

void testCapacity() {
  int logCapacity = 2; // Initial capacity: 2^2 = 4
  CircularArray array(logCapacity);
  if (array.capacity() == 4) {
    std::cout << "testCapacity: PASS\n";
  } else {
    std::cout << "testCapacity: FAIL (expected 4, got " << array.capacity() << ")\n";
  }
}

void testPutAndGet() {
  CircularArray array(2);

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2"; });

  array.put(0, task1);
  array.put(1, task2);

  if (array.get(0) == task1 && array.get(1) == task2) {
    std::cout << "testPutAndGet: PASS\n";
  } else {
    std::cout << "testPutAndGet: FAIL\n";
  }
}

void testCircularIndexing() {
  CircularArray array(2);
  auto task = std::make_shared<std::function<void()>>([]() { std::cout << "Task"; });

  int capacity = array.capacity();
  array.put(capacity, task); // Put at index equal to capacity

  if (array.get(0) == task) {
    std::cout << "testCircularIndexing: PASS\n";
  } else {
    std::cout << "testCircularIndexing: FAIL\n";
  }
}

void testResize() {
  CircularArray array(2);

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2"; });

  array.put(0, task1);
  array.put(1, task2);

  auto resizedArray = array.resize(2, 0);

  if (resizedArray->capacity() == 8 && resizedArray->get(0) == task1 && resizedArray->get(1) == task2) {
    std::cout << "testResize: PASS\n";
  } else {
    std::cout << "testResize: FAIL\n";
  }
}

void testResizeWithWraparound() {
  CircularArray array(2);

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2"; });

  int capacity = array.capacity();

  array.put(capacity - 1, task1); // Last index before wraparound
  array.put(capacity, task2);     // Wraparound to index 0

  auto resizedArray = array.resize(capacity + 1, capacity - 1);

  if (resizedArray->capacity() == 8 && resizedArray->get(capacity - 1) == task1 && resizedArray->get(capacity) == task2) {
    std::cout << "testResizeWithWraparound: PASS\n";
  } else {
    std::cout << "testResizeWithWraparound: FAIL\n";
  }
}

void testIsEmpty() {
  UnboundedDEQueue queue;
  if (queue.isEmpty()) {
    std::cout << "testIsEmpty (initial): PASS\n";
  } else {
    std::cout << "testIsEmpty (initial): FAIL\n";
  }

  auto task1 = std::make_shared<std::function<void()>>([]() {});
  queue.pushBottom(task1);

  if (!queue.isEmpty()) {
    std::cout << "testIsEmpty (after push): PASS\n";
  } else {
    std::cout << "testIsEmpty (after push): FAIL\n";
  }

  queue.popTop(); // Remove the task
  if (queue.isEmpty()) {
    std::cout << "testIsEmpty (after pop): PASS\n";
  } else {
    std::cout << "testIsEmpty (after pop): FAIL\n";
  }
}

void testPushBottomAndPopTop() {
  UnboundedDEQueue queue;

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1\n"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2\n"; });

  queue.pushBottom(task1);
  queue.pushBottom(task2);

  auto poppedTask1 = queue.popTop();
  auto poppedTask2 = queue.popTop();

  if (poppedTask1 == task1 && poppedTask2 == task2) {
    std::cout << "testPushBottomAndPopTop: PASS\n";
  } else {
    std::cout << "testPushBottomAndPopTop: FAIL\n";
  }
}

void testPushBottomAndPopBottom() {
  UnboundedDEQueue queue;

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1\n"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2\n"; });

  queue.pushBottom(task1);
  queue.pushBottom(task2);

  auto poppedTask2 = queue.popBottom();
  auto poppedTask1 = queue.popBottom();

  if (poppedTask2 == task2 && poppedTask1 == task1) {
    std::cout << "testPushBottomAndPopBottom: PASS\n";
  } else {
    std::cout << "testPushBottomAndPopBottom: FAIL\n";
  }
}

void testPopFromEmptyQueue() {
  UnboundedDEQueue queue;
  auto task = queue.popTop(); // Should return nullptr
  if (task == nullptr) {
    std::cout << "testPopFromEmptyQueue (top): PASS\n";
  } else {
    std::cout << "testPopFromEmptyQueue (top): FAIL\n";
  }

  task = queue.popBottom(); // Should also return nullptr
  if (task == nullptr) {
    std::cout << "testPopFromEmptyQueue (bottom): PASS\n";
  } else {
    std::cout << "testPopFromEmptyQueue (bottom): FAIL\n";
  }
}

void testConcurrentPushPop() {
  UnboundedDEQueue queue;

  auto task1 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 1\n"; });
  auto task2 = std::make_shared<std::function<void()>>([]() { std::cout << "Task 2\n"; });

  // Simulate concurrent push and pop
  queue.pushBottom(task1);
  auto poppedTask = queue.popTop();

  if (poppedTask == task1) {
    std::cout << "testConcurrentPushPop: PASS\n";
  } else {
    std::cout << "testConcurrentPushPop: FAIL\n";
  }
}

void testThreadPoolBasicFunctionality() {
  ThreadPool pool(4);          // Create a thread pool with 4 threads
  std::atomic<int> counter(0); // Shared counter

  // Enqueue 10 tasks
  for (int i = 0; i < 10; ++i) {
    pool.Enqueue([&counter]() { counter.fetch_add(1); });
  }

  pool.Wait(); // Wait for all tasks to complete

  // Check if the counter matches the number of tasks
  if (counter.load() == 10) {
    std::cout << "testThreadPoolBasicFunctionality: PASS\n";
  } else {
    std::cout << "testThreadPoolBasicFunctionality: FAIL (expected 10, got " << counter.load() << ")\n";
  }
}

void testThreadPoolWithMultipleTasks() {
  ThreadPool pool(1);
  std::atomic<int> counter(0);

  const int n = 310;
  // Enqueue 100 tasks
  for (int i = 0; i < n; ++i) {
    pool.Enqueue([&counter]() { counter.fetch_add(1); });
  }

  pool.Wait();

  if (counter.load() == n) {
    std::cout << "testThreadPoolWithMultipleTasks: PASS\n";
  } else {
    std::cout << "testThreadPoolWithMultipleTasks: FAIL (expected 100, got " << counter.load() << ")\n";
  }
}

void testThreadPoolShutdown() {
  ThreadPool pool(4);
  std::atomic<int> counter(0);
  auto task = [&counter]() { counter.fetch_add(1); };

  // Enqueue 10 tasks
  for (int i = 0; i < 10; ++i) {
    pool.Enqueue(task);
  }

  // Wait for all tasks to complete before destroying the pool
  pool.Wait();

  if (counter.load() == 10) {
    std::cout << "testThreadPoolShutdown: PASS\n";
  } else {
    std::cout << "testThreadPoolShutdown: FAIL (expected 10, got " << counter.load() << ")\n";
  }
}

void testThreadPoolTaskStealing() {
  ThreadPool pool(4);
  std::atomic<int> counter(0);

  // Create tasks that increment the counter
  for (int i = 0; i < 20; ++i) {
    pool.Enqueue([&counter]() { counter.fetch_add(1); });
  }

  pool.Wait();

  if (counter.load() == 20) {
    std::cout << "testThreadPoolTaskStealing: PASS\n";
  } else {
    std::cout << "testThreadPoolTaskStealing: FAIL (expected 20, got " << counter.load() << ")\n";
  }
}

void testThreadPoolConcurrentEnqueue() {
  ThreadPool pool(4);
  std::atomic<int> counter(0);
  std::vector<std::thread> threads;

  // Launch several threads that enqueue tasks
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&pool, &counter]() {
      for (int j = 0; j < 10; ++j) {
        pool.Enqueue([&counter]() { counter.fetch_add(1); });
      }
    });
  }

  // Join all threads
  for (auto &thread : threads) {
    thread.join();
  }

  pool.Wait();

  if (counter.load() == 100) {
    std::cout << "testThreadPoolConcurrentEnqueue: PASS\n";
  } else {
    std::cout << "testThreadPoolConcurrentEnqueue: FAIL (expected 100, got " << counter.load() << ")\n";
  }
}

} // namespace work_stealing

int main() {
  using namespace work_stealing;
  testCapacity();
  testPutAndGet();
  testCircularIndexing();
  testResize();
  testResizeWithWraparound();

  testIsEmpty();
  testPushBottomAndPopTop();
  testPushBottomAndPopBottom();
  testPopFromEmptyQueue();
  testConcurrentPushPop();

  // testThreadPoolBasicFunctionality();
  testThreadPoolWithMultipleTasks();
  // testThreadPoolShutdown();
  // testThreadPoolTaskStealing();
  // testThreadPoolConcurrentEnqueue();
  return 0;
}
