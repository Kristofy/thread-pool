#pragma once
#include <stdint.h>

void verify_collatz_until(uint64_t n, bool colored_output = true, bool progress_update = true);

uint64_t get_collatz_checksum_until(uint64_t n, bool colored_output = true, bool progress_update = true);

int16_t collatz(uint64_t n);
