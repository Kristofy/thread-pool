#pragma once

#include <string>

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

inline std::string colorTerminal(TerminalColor tc, bool bold = false) {
  std::string tc_num = std::to_string(static_cast<int>(tc));

  return bold ? "\033[1;" + tc_num + "m" : "\033[" + tc_num + "m";
}

inline std::string resetTerminal() {
  std::string color = {
      '\033', '[', '0', 'm', '\0'
  };

  return color;
}
