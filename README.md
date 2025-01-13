# Thread Pool with Work Stealing Queue

This project implements a thread pool with a work-stealing queue, along with benchmarks and tests.

## Usage

![./docs/example.gif](./docs/example.gif)

### Prerequisites

- C++17 compiler (Configure in Makefile if not gcc)
- Make
- (Optional) Address Sanitizer (gcc -fsanitize=address)
- (Optional) A terminal emulator that supports unicode characters and colors (e.g. Alacritty)

### Build and run

By default `make` does a debug build, use `make release=1` for a release build flags.
By default `make` does not include address sanitizer, use `make asan=1` to include it. (for debug builds only)

```bash
make release=1 run # This builds and runs the program in release mode

# To only build
make # Or in release mode `make release=1`

# To only run
./main
```

### Tests

For the tests the flags are the same

```bash
make test # This builds and runs the tests

make asan=1 test # This builds and runs the tests with address sanitizer

make btest # This builds the tests

./tests # This runs the tests
```

If you have to force a rebuild use `make clean`

## Configuration

Open the `./main.cpp` file and comment in / out the relevant benchmarks in the `main` function

For example

```cpp
 std::vector<std::tuple<std::string, std::function<std::vector<int16_t>(uint64_t)>>> functions = {
      {"Naive", naive},
      {"LockingTP 100b / 2t", locking_threadpool<n_max / 100, 2>},
      {"LockingTP 10000b / 6t", locking_threadpool<n_max / 10000, 6>},
      {"Ws_CAS_TP 100b / 2t", work_stealing_threadpool<n_max / 100, 2>},
      {"Ws_CAS_TP 10000b / 6t", work_stealing_threadpool<n_max / 10000, 6>},
  };
```

This means that the Naive, Threadpool with Blocking Queue (LockingTP) and Threadpool with Work Stealing Queue (Ws_CAS_TP) will be benchmarked.
The Blocking queue and the Work Stealing queue will have a 100 block size and 2 threads and 10000 block size and 6 threads versions. Where 100 or 100000 small tasks are bundled togethere in a block, this essentially means the number of "work" in a single task.

## Example Results

An output should be interactively rendered like this

```
$ make release=1 run
Benchmark #1: Naive
        Time (mean ± σ):                131.60 ms ± 5.44 ms
        Range (min … max):              126.90 ms … 144.68 ms

Benchmark #2: LockingTP 100b / 2t
        Time (mean ± σ):                100.12 ms ± 31.76 ms
        Range (min … max):              66.59 ms … 144.68 ms

Benchmark #3: LockingTP 10000b / 6t
        Time (mean ± σ):                78.68 ms ± 39.91 ms
        Range (min … max):              34.67 ms … 144.68 ms

Benchmark #4: Ws_CAS_TP 100b / 2t
        Time (mean ± σ):                85.21 ms ± 36.51 ms
        Range (min … max):              34.67 ms … 144.68 ms

Benchmark #5: Ws_CAS_TP 10000b / 6t
        Time (mean ± σ):                88.20 ms ± 33.22 ms
        Range (min … max):              34.67 ms … 144.68 ms

Summary:
        'LockingTP 10000b / 6t' ran
        1.08 ± 0.61 times faster than 'Ws_CAS_TP 100b / 2t';
        1.12 ± 0.56 times faster than 'Ws_CAS_TP 10000b / 6t';
        1.27 ± 0.47 times faster than 'LockingTP 100b / 2t';
        1.67 ± 0.30 times faster than 'Naive';
```

Here you can see the average time and standard deviation for each of the benchmarked functions (of 10 runs by default).

The summary at the end shows the relative performance of each function with respect to the first one, as well as the standard deviation of the relative performance. The results are also ranked from fastest to slowest.

Here `LockingTP 10000b / 6t` is 1.08 times faster than `Ws_CAS_TP 100b / 2t`, 1.12 times faster than `Ws_CAS_TP 10000b / 6t`, 1.27 times faster than `LockingTP 100b / 2t` and 1.67 times faster than `Naive`.

## Results

Run on:

- fedora-41-x86_64 (GNU/Linux)
- gcc (GCC) 14.2.1 20240912 (Red Hat 14.2.1-3)
- Ryzen 7 5800U 8C / 16T Processor
- 16 GB 4266 MT/s Dual Channel LPDDR4

**Results by thread count**

Fastest to slowest

- 'Ws_CAS_TP 1000b / 16t' ran
- 1.35 ± 0.29 times faster than 'Ws_CAS_TP 1000b / 32t';
- 1.42 ± 0.11 times faster than 'Ws_CAS_TP 1000b / 8t';
- 1.60 ± 0.11 times faster than 'Ws_CAS_TP 1000b / 64t';
- 2.10 ± 0.05 times faster than 'Ws_CAS_TP 1000b / 4t';
- 3.60 ± 0.03 times faster than 'Ws_CAS_TP 1000b / 2t';
- 7.04 ± 0.02 times faster than 'Ws_CAS_TP 1000b / 1t';

Comparing with other implementation and other configurations with block sizes of 100, 1000 and 10000 and threads of 4, 6, 8, 16, 32 and 64 the results are as follows:

| Benchmark                  | Time (mean ± σ)     | Range (min … max)     |
| -------------------------- | ------------------- | --------------------- |
| **Naive**                  | 126.39 ms ± 4.07 ms | 123.94 ms … 138.15 ms |
| **LockingTP 100b / 4t**    | 39.24 ms ± 1.62 ms  | 37.96 ms … 43.01 ms   |
| **LockingTP 1000b / 4t**   | 38.90 ms ± 0.43 ms  | 38.22 ms … 39.55 ms   |
| **LockingTP 10000b / 4t**  | 45.86 ms ± 0.61 ms  | 45.20 ms … 46.87 ms   |
| **Ws_CAS_TP 100b / 4t**    | 38.57 ms ± 0.51 ms  | 37.94 ms … 39.42 ms   |
| **Ws_CAS_TP 1000b / 4t**   | 39.05 ms ± 1.44 ms  | 37.15 ms … 41.77 ms   |
| **Ws_CAS_TP 10000b / 4t**  | 41.52 ms ± 3.30 ms  | 39.33 ms … 50.92 ms   |
| **LockingTP 100b / 6t**    | 29.27 ms ± 0.39 ms  | 28.61 ms … 29.93 ms   |
| **LockingTP 1000b / 6t**   | 29.64 ms ± 0.86 ms  | 29.01 ms … 32.00 ms   |
| **LockingTP 10000b / 6t**  | 35.68 ms ± 0.86 ms  | 34.40 ms … 37.07 ms   |
| **Ws_CAS_TP 100b / 6t**    | 31.80 ms ± 3.75 ms  | 28.51 ms … 40.88 ms   |
| **Ws_CAS_TP 1000b / 6t**   | 28.80 ms ± 0.78 ms  | 28.04 ms … 30.62 ms   |
| **Ws_CAS_TP 10000b / 6t**  | 29.88 ms ± 0.86 ms  | 29.16 ms … 32.26 ms   |
| **LockingTP 100b / 8t**    | 25.08 ms ± 1.00 ms  | 23.92 ms … 26.70 ms   |
| **LockingTP 1000b / 8t**   | 24.46 ms ± 0.61 ms  | 23.80 ms … 25.90 ms   |
| **LockingTP 10000b / 8t**  | 30.43 ms ± 1.04 ms  | 29.36 ms … 32.73 ms   |
| **Ws_CAS_TP 100b / 8t**    | 24.46 ms ± 0.32 ms  | 23.98 ms … 24.93 ms   |
| **Ws_CAS_TP 1000b / 8t**   | 23.26 ms ± 0.21 ms  | 22.99 ms … 23.68 ms   |
| **Ws_CAS_TP 10000b / 8t**  | 24.64 ms ± 0.31 ms  | 24.25 ms … 25.31 ms   |
| **LockingTP 100b / 16t**   | 17.69 ms ± 0.39 ms  | 17.23 ms … 18.53 ms   |
| **LockingTP 1000b / 16t**  | 20.09 ms ± 2.62 ms  | 17.64 ms … 25.38 ms   |
| **LockingTP 10000b / 16t** | 20.56 ms ± 1.20 ms  | 19.21 ms … 23.67 ms   |
| **Ws_CAS_TP 100b / 16t**   | 21.84 ms ± 2.04 ms  | 18.70 ms … 25.97 ms   |
| **Ws_CAS_TP 1000b / 16t**  | 17.85 ms ± 0.57 ms  | 17.06 ms … 19.10 ms   |
| **Ws_CAS_TP 10000b / 16t** | 18.68 ms ± 0.43 ms  | 17.97 ms … 19.47 ms   |
| **LockingTP 100b / 32t**   | 18.22 ms ± 0.51 ms  | 17.09 ms … 18.94 ms   |
| **LockingTP 1000b / 32t**  | 17.46 ms ± 0.34 ms  | 17.07 ms … 18.00 ms   |
| **LockingTP 10000b / 32t** | 20.25 ms ± 1.18 ms  | 18.86 ms … 23.11 ms   |
| **Ws_CAS_TP 100b / 32t**   | 24.63 ms ± 2.04 ms  | 22.39 ms … 29.71 ms   |
| **Ws_CAS_TP 1000b / 32t**  | 20.65 ms ± 1.40 ms  | 18.73 ms … 23.48 ms   |
| **Ws_CAS_TP 10000b / 32t** | 23.67 ms ± 2.23 ms  | 21.01 ms … 27.72 ms   |
| **LockingTP 100b / 64t**   | 23.26 ms ± 2.60 ms  | 19.97 ms … 28.93 ms   |
| **LockingTP 1000b / 64t**  | 23.64 ms ± 2.52 ms  | 19.68 ms … 27.22 ms   |
| **LockingTP 10000b / 64t** | 43.18 ms ± 7.21 ms  | 29.12 ms … 51.38 ms   |
| **Ws_CAS_TP 100b / 64t**   | 37.47 ms ± 8.34 ms  | 29.44 ms … 58.74 ms   |
| **Ws_CAS_TP 1000b / 64t**  | 30.14 ms ± 3.81 ms  | 23.38 ms … 39.41 ms   |
| **Ws_CAS_TP 10000b / 64t** | 33.19 ms ± 4.54 ms  | 26.17 ms … 39.45 ms   |

### Summary (Baseline(Fastest): `LockingTP 10000b / 6t`)

- **LockingTP 1000b / 32t** ran:
  - 1.01 ± 0.03 times faster than **LockingTP 100b / 16t**;
  - 1.02 ± 0.04 times faster than **Ws_CAS_TP 1000b / 16t**;
  - 1.04 ± 0.03 times faster than **LockingTP 100b / 32t**;
  - 1.07 ± 0.03 times faster than **Ws_CAS_TP 10000b / 16t**;
  - 1.15 ± 0.11 times faster than **LockingTP 1000b / 16t**;
  - 1.16 ± 0.05 times faster than **LockingTP 10000b / 32t**;
  - 1.18 ± 0.05 times faster than **LockingTP 10000b / 16t**;
  - 1.18 ± 0.06 times faster than **Ws_CAS_TP 1000b / 32t**;
  - 1.25 ± 0.08 times faster than **Ws_CAS_TP 100b / 16t**;
  - 1.33 ± 0.09 times faster than **LockingTP 100b / 64t**;
  - 1.33 ± 0.02 times faster than **Ws_CAS_TP 1000b / 8t**;
  - 1.35 ± 0.08 times faster than **LockingTP 1000b / 64t**;
  - 1.36 ± 0.07 times faster than **Ws_CAS_TP 10000b / 32t**;
  - 1.40 ± 0.02 times faster than **LockingTP 1000b / 8t**;
  - 1.40 ± 0.03 times faster than **LockingTP 100b / 64t**;
  - 1.46 ± 0.03 times faster than **LockingTP 10000b / 8t**;
  - 1.48 ± 0.04 times faster than **LockingTP 100b / 4t**;
  - 1.62 ± 0.04 times faster than **LockingTP 1000b / 4t**;
  - 1.65 ± 0.02 times faster than **LockingTP 10000b / 4t**;
  - 1.66 ± 0.06 times faster than **Ws_CAS_TP 10000b / 4t**;
  - 1.67 ± 0.03 times faster than **LockingTP 100b / 8t**;
  - 1.73 ± 0.03 times faster than **LockingTP 100b / 100t**;
  - 1.78 ± 0.04 times faster than **LockingTP 100b / 1000t**;
  - 1.78 ± 0.01 times faster than **LockingTP 100b / 10000t**;

### The benchmarked tasks

How many steps does it take for the function intruduced in the [collatz conjecture](https://en.wikipedia.org/wiki/Collatz_conjecture) to reach 1?

It is empiricaly shown that for the maximum uint32 it ends on one from any starting number.
Each benchmark runs the collatz function to count the steps to reach 1 for every starting number from 1 to 1000000.
t
The block size 100 means that 100 calculactions are bundled together in a block, resulting in 10000 tasks in total.

The running time of the benchmarks are slower than the reported measurements, because every calculation is the verified by the main thread.

## Notes

- The reference blocking queue implementation is also my own and not optimized.
- The reference work stealing queue implementation is also my own and not optimized and is based on the implementation described in the book [#](The Art of Multiprocessor Programming by Maurice Herlihy and Nir Shavit).
- The implemented work stealing queue is unbounded but only supports a single consumer and a single producer from the **same thread** and an additional consumer from any number of different threads (the currently stealing thread).
- The work stealing queue is slower than the blocking queue, but it does support more operations (stealing) and is lock-free, so the progress of a single thread is garanteed. (We do use locks for the initial syncronization phase, but not during the execution of the tasks)
- We use a "Unity" build for easier and faster compilation, meaning the source files are included in the main source files.
- We do check if the benchmark is safe, meaning during the iteration we do not over or underflow the integer type.
- To get a build configuration for the IDE you can run `make configuration` given that you have `bear` installed

## Acknowledgements

The colortheme and output layout is based on [https://github.com/sharkdp/hyperfine](https://github.com/sharkdp/hyperfine)

This project is my assignment for the Parallel Programming course at the University of Elte

## Future Improvements

Right now to use CAS for the work stealing queue it is convenient to use pointers for tasks wrapped in std::functional, but a much more memory and compute efficient way to solve this would be to reuse memory and store the tasks in a contiguous array (not an array of pointers, but the data itself), which would be much more efficient. but the cas operation would be much harder to use on parameteres bigger than the work size.
