#pragma once

#include "colors.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct statistics {
  std::string name;
  double mean_ms;
  double standard_deviation_ms;
  double min;
  double max;

  static statistics from_vector(const std::vector<double> &times, const std::string &name) {
    statistics stats;
    stats.name = name;

    stats.mean_ms               = std::accumulate(times.begin(), times.end(), 0.0) / static_cast<double>(times.size());
    stats.standard_deviation_ms = 0.0;
    for (const auto &time : times) {
      stats.standard_deviation_ms += std::pow(time - stats.mean_ms, 2);
    }
    stats.standard_deviation_ms = std::sqrt(stats.standard_deviation_ms / static_cast<double>(times.size()));
    stats.min                   = *std::min_element(times.begin(), times.end());
    stats.max                   = *std::max_element(times.begin(), times.end());

    return stats;
  }

  static void print_statistics(const std::vector<statistics> &stats) {
    std::cout << std::setprecision(2);
    for (std::size_t i = 0; i < stats.size(); i++) {
      const auto &stat = stats[i];
      std::cout << colorTerminal(TerminalColor::TC_BRIGHT_WHITE, true)
                << "Benchmark #" << i + 1 << ": " << stat.name << '\n'
                << resetTerminal()
                << "\tTime (" << colorTerminal(TerminalColor::TC_GREEN, true) << "mean" << resetTerminal() << " ± " << colorTerminal(TerminalColor::TC_GREEN) << "σ" << resetTerminal() << "):\t\t" << colorTerminal(TerminalColor::TC_GREEN, true) << stat.mean_ms << " ms" << resetTerminal() << " ± " << colorTerminal(TerminalColor::TC_GREEN) << stat.standard_deviation_ms << " ms" << resetTerminal() << "\n"
                << "\tRange (" << colorTerminal(TerminalColor::TC_CYAN, true) << "min" << resetTerminal() << " … " << colorTerminal(TerminalColor::TC_RED, true) << "max" << resetTerminal() << "):\t\t" << colorTerminal(TerminalColor::TC_CYAN, true) << stat.min << " ms" << resetTerminal() << " … " << colorTerminal(TerminalColor::TC_RED, true) << stat.max << " ms" << resetTerminal() << "\n\n";
    }

    if (stats.size() < 2) {
      return;
    }

    // Compute the summary
    auto fastest_index_offset = std::min_element(stats.begin(), stats.end(), [](const statistics &lhs, const statistics &rhs) {
                                  return lhs.mean_ms < rhs.mean_ms || (lhs.mean_ms == rhs.mean_ms && lhs.standard_deviation_ms < rhs.standard_deviation_ms);
                                }) -
                                stats.begin();
    uint64_t fastest_index = static_cast<uint64_t>(fastest_index_offset);

    std::vector<std::tuple<double, double, std::string>> relative_speeds(stats.size() - 1);
    for (std::size_t i = 0; i < stats.size(); i++) {
      if (i == fastest_index) {
        continue;
      }

      const auto &stat    = stats[i];
      const auto &fastest = stats[fastest_index];
      const double ratio  = stat.mean_ms / fastest.mean_ms;

      relative_speeds[i - (i > fastest_index)] = {
          // How much faster is the fastest the this one on average
          ratio,

          // https://en.wikipedia.org/wiki/Propagation_of_uncertainty#Example_formulae
          // Covariance asssumed to be 0, i.e. variables are assumed to be independent
          1.0 / ratio * std::sqrt(std::pow(stat.standard_deviation_ms / stat.mean_ms, 2.0) + std::pow(fastest.standard_deviation_ms / fastest.mean_ms, 2)),

          // Name of the benchmark
          stat.name,
      };
    }

    std::sort(relative_speeds.begin(), relative_speeds.end());

    std::cout << colorTerminal(TerminalColor::TC_BRIGHT_WHITE, true) << "Summary: " << resetTerminal() << '\n';
    std::cout << "\t'" << colorTerminal(TerminalColor::TC_CYAN, true) << stats[fastest_index].name << resetTerminal() << "' ran\n";

    for (const auto &[mean_ratio, stddev, name] : relative_speeds) {
      std::cout << "\t" << colorTerminal(TerminalColor::TC_GREEN, true) << std::fixed << std::setprecision(2) << mean_ratio << resetTerminal() << " ± " << colorTerminal(TerminalColor::TC_GREEN) << stddev << resetTerminal() << " times faster than '" << colorTerminal(TerminalColor::TC_RED, true) << name << resetTerminal() << "';\n";
    }
  }
};
