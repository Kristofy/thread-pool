#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <future>
#include <condition_variable>

namespace compare_exchange {

// Lock-Free Queue (as in previous implementation)
template <typename T>
class LockFreeQueue {
private:
  struct Node {
    T data;
    std::atomic<Node *> next;

    Node(T val) : data(val), next(nullptr) {}
  };

  std::atomic<Node *> head;
  std::atomic<Node *> tail;

public:
  LockFreeQueue() {
    Node *dummy = new Node(T{}); // Create a dummy node
    head.store(dummy);
    tail.store(dummy);
  }

  ~LockFreeQueue() {
    while (head.load() != nullptr) {
      Node *temp = head.load();
      head.store(head.load()->next.load());
      delete temp;
    }
  }

  void push(const T &value) {
    Node *newNode = new Node(value); // Allocate new node
    Node *oldTail = nullptr;

    while (true) {
      oldTail    = tail.load(); // Load current tail
      Node *next = oldTail->next.load();

      if (oldTail == tail.load()) { // Ensure tail has not changed
        if (next == nullptr) {      // Tail is indeed the last node
          if (oldTail->next.compare_exchange_weak(next, newNode)) {
            tail.compare_exchange_weak(oldTail, newNode);
            return;
          }
        } else {
          tail.compare_exchange_weak(oldTail, next);
        }
      }
    }
  }

  bool pull(T &result) {
    Node *oldHead = nullptr;

    while (true) {
      oldHead       = head.load(); // Load the current head
      Node *oldTail = tail.load();
      Node *next    = oldHead->next.load();

      if (oldHead == head.load()) { // Ensure head has not changed
        if (oldHead == oldTail) {
          if (next == nullptr) { // Queue is empty
            return false;
          }
          tail.compare_exchange_weak(oldTail, next);
        } else {
          if (head.compare_exchange_weak(oldHead, next)) {
            result = next->data;
            // delete oldHead; // Cleanup old head node
            return true;
          }
        }
      }
    }
  }
};

// Thread Pool Implementation
class ThreadPool {
private:
  LockFreeQueue<std::function<void()>> taskQueue;
  std::vector<std::thread> workers;
  std::atomic<bool> stop;
  std::atomic<int> activeTasks;
  std::condition_variable_any cvWait;
  std::mutex waitMutex;

public:
  ThreadPool(size_t threadCount) : stop(false), activeTasks(0) {
    for (size_t i = 0; i < threadCount; ++i) {
      workers.emplace_back([this]() { this->workerThread(); });
    }
  }

  ~ThreadPool() {
    stop.store(true);
    for (std::thread &worker : workers) {
      if (worker.joinable())
        worker.join();
    }
  }

  template <typename F, typename... Args>
  void Enqueue(F &&func, Args &&...args) {
    taskQueue.push([func, args...]() { func(args...); });
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(waitMutex);
    cvWait.wait(lock, [this]() { return taskQueueIsEmpty() && activeTasks.load() == 0; });
  }

private:
  void workerThread() {
    while (!stop.load()) {
      std::function<void()> task;
      if (taskQueue.pull(task)) {
        activeTasks.fetch_add(1);
        task();
        activeTasks.fetch_sub(1);
        cvWait.notify_all();
      } else {
        std::this_thread::yield();
      }
    }
  }

  bool taskQueueIsEmpty() {
    std::function<void()> tempTask;
    return !taskQueue.pull(tempTask);
  }
};

} // namespace compare_exchange
