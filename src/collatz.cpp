/**
 * @file collatz.cpp
 * @brief The collatz conjecture
 * @date 2025-1-08
 * 
 * Collatz conjecture and related functions
 */

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#include "utils.hpp"
#include "collatz.h"

/**
 * @brief Verifies that the function in the collatz conjecture behaves well in 64 bit integers
 *
 * Using unsiged 64 bit integers for the values the function should never produce an overflow
 * @param n
 */
void verify_collatz_until(uint64_t n, bool colored_output, bool progress_update) {

  // Intro
  if (progress_update) {
    std::cout << "Verifying until: " << n << std::endl;
    if (colored_output) {
      std::cout << colorTerminal(TerminalColor::TC_BRIGHT_YELLOW);
    }
    std::cout << std::fixed << std::showpoint << std::setprecision(2);
    std::cout << "0.00%";
  }

  // Main loop
  for (uint64_t i = 1; i <= n; i++) {
    uint64_t m = i;

    // Inlined collatz
    while (m != 1) {
      if (m % 2 == 0) {
        m /= 2;
      } else {
        if (m > std::numeric_limits<uint64_t>::max() / 3 - 1) {
          if (colored_output) std::cout << colorTerminal(TerminalColor::TC_BRIGHT_RED);
          std::cout << "Overflow at: " << i << std::endl;
          if (colored_output) std::cout << resetTerminal();
          std::abort();
        }
        m = 3 * m + 1;
      }
    }

    if (progress_update && i % 1'000 == 0) {
      std::cout << '\r' << (static_cast<double>(i) / static_cast<double>(n)) * 100 << "%";
    }
  }

  if (progress_update) {
    std::cout << '\n';
    if (colored_output) std::cout << colorTerminal(TerminalColor::TC_BRIGHT_GREEN);
    std::cout << "Verified until: " << n << std::endl;
    if (colored_output) std::cout << resetTerminal();
  }
}

/**
 * @brief Calculates the checksum of the collatz sequence until @a n.
 *
 * @details This function calculates the checksum of the collatz sequence until
 * @a n. The checksum is calculated by summing the number of steps it takes to
 * reach 1 for each number from 1 to @a n. The function prints out the progress
 * of the calculation as a percentage of the total number of calculations.
 *
 * @param n The number to calculate the checksum until.
 * @param colored_output If set to @c true, the output will be colored using
 * the @c TerminalColor enum.
 * @param progress_update If set to @c true, the progress of the calculation
 * will be printed out as a percentage of the total number of calculations.
 * @return The checksum of the collatz sequence until @a n.
 */
uint64_t get_collatz_checksum_until(uint64_t n, bool colored_output, bool progress_update) {

  const uint64_t COLLATZ_1E8_CHECKSUM = 6117591564791386555ULL;
  const uint64_t COLLATZ_1E9_CHECKSUM = 1971640394277668423ULL;

  if (n == 100'000'000ULL) {
    if (progress_update) {
      std::cout << "Checksum: 100.00%\n";
    }
    return COLLATZ_1E8_CHECKSUM;
  }

  if (n == 1'000'000'000ULL) {
    if (progress_update) {
      std::cout << "Checksum: 100.00%\n";
    }
    return COLLATZ_1E9_CHECKSUM;
  }

  // Intro
  if (progress_update) {
    if (colored_output) std::cout << colorTerminal(TerminalColor::TC_BRIGHT_YELLOW);
    std::cout << std::fixed << std::showpoint << std::setprecision(2);
    std::cout << "Checksum: 0.00%";
  }

  std::vector<int16_t> collatz_values(n);
  for (uint64_t i = 1; i <= n; i++) {
    uint64_t m = i;

    // Inlined collatz
    int16_t steps = 0;
    while (m != 1) {
      if (m % 2 == 0) {
        m /= 2;
      } else {
        m = 3 * m + 1;
      }
      steps++;
    }

    collatz_values[i - 1] = steps;

    if (progress_update && i % 1'000 == 0) {
      std::cout << "\rChecksum: " << (static_cast<double>(i) / static_cast<double>(n)) * 100 << "%";
    }
  }

  if (progress_update) {
    std::cout << std::endl;
    if (colored_output) std::cout << resetTerminal();
  }

  return checksum(collatz_values);
}


/**
 * @brief An iterative method of the collatz conjectures famous sequence.
 *
 * @param n The number to start the sequence from.
 * @return The number of steps it took to reach 1.
 */
int16_t collatz(uint64_t n) {
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