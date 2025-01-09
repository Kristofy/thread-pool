#pragma once

#include <cstdint>

/**
 * @brief Good enough checksum
 *
 * @param shorts
 * @return checksum
 */
template <typename Container>
inline uint64_t checksum(const Container &shorts) {
  uint64_t checksum    = 0;
  const uint64_t prime = 31;

  for (int16_t value : shorts) {
    checksum ^= static_cast<uint64_t>(value) * prime;    // Mix the value into the checksum
    checksum = (checksum << 5) | (checksum >> (64 - 5)); // Rotate left by 5 bits
  }

  return checksum;
}

#define DO_NOT_OPTIMIZE(expr) asm volatile("" : : "r,m"(expr) : "memory")
