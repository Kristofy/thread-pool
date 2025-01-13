/**
 * @file tests.cpp
 * @brief Unit tests for the WorkStealingQueue class and components
 * @date 2025-1-08
 * 
 * @note You can run this with the command `make test`
 */

#include <atomic>
#include <iostream>
#include <memory>
#include <functional>
#include "thread_pools/work_stealing_queue.hpp" // Assume the class is in this header file

// Unity build
#include "thread_pools/work_stealing_queue.cpp"


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
  delete resizedArray;
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

  delete resizedArray;
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

void testThreadPoolWithMultipleTasks() {
  work_stealing::ThreadPool tp(4);
  std::vector<std::function<void()>> tasks;

  std::atomic<uint64_t> cnt = 0;
  for (uint64_t i = 0; i < tp.queues.size(); i++) {

    tasks.push_back([&tp, &cnt]() {
     
        tp.queues[work_stealing::me]->pushBottom(std::make_shared<std::function<void()>>([&cnt]() {
          ++cnt;
        }));
        tp.task_count.fetch_add(1, std::memory_order_relaxed);
    });
  }

  tp.Enqueue(std::move(tasks));

  tp.Wait();

  if (cnt == tp.queues.size()) {
    std::cout << "testThreadPoolWithMultipleTasks: PASS\n";
  } else {
    std::cout << "testThreadPoolWithMultipleTasks: FAIL\n";
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

  testThreadPoolWithMultipleTasks();
  return 0;
}
