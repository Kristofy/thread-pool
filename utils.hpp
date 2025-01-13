#pragma once

#include <cstdint>
#include <string>

/**
 * @brief A list of ANSI terminal colors
 */
enum class TerminalColor : int {
  TC_BLACK          = 30,
  TC_BRIGHT_BLACK   = 90,
  TC_RED            = 31,
  TC_BRIGHT_RED     = 91,
  TC_GREEN          = 32,
  TC_BRIGHT_GREEN   = 92,
  TC_YELLOW         = 33,
  TC_BRIGHT_YELLOW  = 93,
  TC_BLUE           = 34,
  TC_BRIGHT_BLUE    = 94,
  TC_MAGENTA        = 35,
  TC_BRIGHT_MAGENTA = 95,
  TC_CYAN           = 36,
  TC_BRIGHT_CYAN    = 96,
  TC_WHITE          = 37,
  TC_BRIGHT_WHITE   = 97
};

/**
 * @brief Get a string that can be used to set the terminal foreground color and/or make the text bold
 *
 * @param tc The desired terminal color
 * @param bold Whether the text should be bold or not
 * @return A string that can be used to set the terminal color and/or make the text bold
 *
 * The returned string is a valid ANSI escape sequence that sets the foreground color
 * and/or makes the text bold. The string can be used as is, or concatenated with other
 * strings to create more complex ANSI escape sequences.
 *
 * @example
 * std::cout << colorTerminal(TerminalColor::TC_BRIGHT_GREEN, true) << "Hello World!" << resetTerminal() << std::endl;
 * // Output: A green, bold "Hello World!"
 */
inline std::string colorTerminal(TerminalColor tc, bool bold = false) {
  std::string tc_num = std::to_string(static_cast<int>(tc));

  return bold ? "\033[1;" + tc_num + "m" : "\033[" + tc_num + "m";
}

/**
 * @brief Resets the terminal color to the default color
 *
 * @return A string that can be used to reset the terminal color
 *
 * The returned string is a valid ANSI escape sequence that resets the foreground
 * color to the default color. The string can be used as is, or concatenated with
 * other strings to create more complex ANSI escape sequences.
 *
 * @example
 * std::cout << colorTerminal(TerminalColor::TC_BRIGHT_GREEN, true) << "Hello World!" << resetTerminal() << std::endl;
 * // Output: A green, bold "Hello World!" followed by the default terminal color
 */
inline std::string resetTerminal() {
  std::string color = {
      '\033', '[', '0', 'm', '\0'
  };

  return color;
}


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
