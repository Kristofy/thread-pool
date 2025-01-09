/**
 * @file fast_workstealing_threadpool.hpp
 * @brief Implementation of a fast work-stealing thread pool for the AMD64
 * architecture.
 *
 * This header file contains the declaration of a fast work-stealing thread pool
 * designed to efficiently manage and distribute tasks across multiple threads
 * on AMD64 architecture.
 *
 * @date 2024-10-22
 * @author
 * Osztopáni Kristóf (GitHub: Kristofy)
 * kristofosztopani@gmail.com
 *
 * @note This implementation is optimized for the AMD64 architecture.
 */

#pragma once

#include <cstdlib> // for std::aligned_alloc
#include <new>     // for cache line alignment
#include <vector>

constexpr auto CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
using index_t                  = unsigned long long;

// Alignes a type to a cache line, and adds padding to avoid false sharing
template <typename T, index_t Align>
class ToAligned {
  static constexpr index_t Padding = (Align - sizeof(T) % Align) % Align;
  struct alignas(Align) Type : public T {
    char _alignment_padding[Padding];
  };
};

// Holdes all the ring buffers and hands them out
template <typename T, unsigned K = 12>
class RingBufferManager {

  struct RingBufferDataImpl {
    const index_t block_index;
    const index_t offset_index;
    const index_t buffer_data_index;
    index_t head;
    index_t tail;
    bool is_full;
  };

  using RingBufferData =
      typename ToAligned<RingBufferDataImpl, CACHE_LINE_SIZE>::Type;

private:
  static constexpr index_t BlockSize = static_cast<index_t>(1) << K;
  static constexpr index_t BlockMask = BlockSize - 1;

public:
  RingBufferManager(index_t num_buffers = 8) // The num buffers is the 2 ^ (ceil(log2(num_buffers)))
      : num_buffers(static_cast<index_t>(1) << (sizeof(index_t) * 8 - __builtin_clzll(num_buffers - 1))),
        ring_buffers_data(static_cast<RingBufferData *>(std::aligned_alloc(CACHE_LINE_SIZE, sizeof(RingBufferData) * num_buffers))) {

    allocated_memory_blocks.reserve(32);
    allocated_memory_blocks.push_back(static_cast<std::byte *>(std::aligned_alloc(CACHE_LINE_SIZE, sizeof(std::byte) * num_buffers * BlockSize)));

    for (index_t i = 0; i < num_buffers; i++) {
      ring_buffers_data = {
          .block_index       = 0,
          .offset_index      = i,
          .buffer_data_index = i,
          .head              = 0,
          .tail              = 0,
      };
      unallocated_buffers.push_back(i);
    }
  }

  ~RingBufferManager() {
    for (auto ptr : allocated_memory_blocks) {
      std::free(ptr);
    }
  }

  // Delete copy constructor and assignment operator
  RingBufferManager(const RingBufferManager &)            = delete;
  RingBufferManager &operator=(const RingBufferManager &) = delete;

  RingBufferManager(RingBufferManager &&)            = delete;
  RingBufferManager &operator=(RingBufferManager &&) = delete;

  class RingBuffer {
  public:
    RingBuffer(RingBufferData &ring_buffer_data, T *buffer)
        : ring_buffer_data_(ring_buffer_data), buffer_(buffer) {}

    const T &back() const;
    void push_back(const T &item);
    void push_back(T &&item);
    T pop_back();

    const T &front() const;
    void push_front(const T &item);
    void push_front(T &&item);
    T pop_front();

    bool is_full_or_empty() const;

  private:
    RingBufferData &ring_buffer_data_;
    T *buffer_;
  };

  RingBuffer allocate_buffer() {
    index_t index;
    if (unallocated_buffers.empty()) {
      // TODO: Allocate new rings, and set up the data for every one of them
    } else {
      index = unallocated_buffers.back();
      unallocated_buffers.pop_back();
    }
    const auto &ring_buffer_data = ring_buffers_data[index];

    ring_buffer_data.head    = 0;
    ring_buffer_data.tail    = 0;
    ring_buffer_data.is_full = false;

    return RingBuffer(ring_buffers_data[index], allocated_memory_blocks[ring_buffer_data.block_index] + BlockSize * ring_buffer_data.offset_index);
  }

  void deallocate_buffer(RingBuffer &ring_buffer) {
    unallocated_buffers.push_back(
        ring_buffer.ring_buffer_data_.buffer_data_index
    );
  }

private:
  index_t num_buffers;
  RingBufferData *ring_buffers_data;
  std::vector<index_t> unallocated_buffers;
  std::vector<std::byte *> allocated_memory_blocks;
};

// A ring buffer has a constant size, and we have multiple of that
template <typename T, unsigned long long N, unsigned Aligned>
class BlockedRingBufferArray {
public:
  // BlockedRingBuffer();

  const T &back() const;
  void push_back(const T &item);
  void push_back(T &&item);
  T pop_back();

  const T &front() const;
  void push_front(const T &item);
  void push_front(T &&item);
  T pop_front();

  bool is_full_or_empty() const;

private:
  T *buffer_;
  unsigned long long head_;
  unsigned long long tail_;
};

template <auto Function>
class ThreadPool {
  static_assert(sizeof(Function) == 0, "ThreadPool must be specialized for Callable types.");
};

template <typename... Args, void (*Function)(Args...) noexcept>
class ThreadPool<Function> {
public:
  /**
   * @brief Submits a task to the thread pool.
   *
   * This function allows you to submit a task with the given arguments to be executed by the thread pool.
   * The task will be executed by one of the available threads in the pool.
   *
   * @param args The arguments to be passed to the task.
   */
  void submit(Args... args) {
    // TODO: Implement
    // at some point Function(std::forward<...Args>(args))
  }
};
