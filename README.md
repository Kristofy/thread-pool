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



For block sizes of 100, 1000 and 10000 and threads of 4, 6, 8, 16, 32 and 64 the results are as follows:

| **Benchmark**                 | **Time (mean ± σ)**   | **Range (min … max)**         |
|-------------------------------|-----------------------|-------------------------------|
| Naive                        | 131.24 ms ± 8.32 ms   | 126.01 ms … 154.93 ms         |
| LockingTP 100b / 4t          | 85.66 ms ± 45.97 ms   | 37.81 ms … 154.93 ms          |
| LockingTP 1000b / 4t         | 70.02 ms ± 43.57 ms   | 37.81 ms … 154.93 ms          |
| LockingTP 10000b / 4t        | 64.07 ms ± 39.12 ms   | 37.81 ms … 154.93 ms          |
| Ws_CAS_TP 100b / 4t          | 70.49 ms ± 37.28 ms   | 37.81 ms … 154.93 ms          |
| Ws_CAS_TP 1000b / 4t         | 74.34 ms ± 35.10 ms   | 37.81 ms … 154.93 ms          |
| Ws_CAS_TP 10000b / 4t        | 77.72 ms ± 33.60 ms   | 37.81 ms … 154.93 ms          |
| LockingTP 100b / 6t          | 71.65 ms ± 35.29 ms   | 28.55 ms … 154.93 ms          |
| LockingTP 1000b / 6t         | 66.92 ms ± 35.86 ms   | 28.55 ms … 154.93 ms          |
| LockingTP 10000b / 6t        | 63.70 ms ± 35.37 ms   | 28.55 ms … 154.93 ms          |
| Ws_CAS_TP 100b / 6t          | 67.02 ms ± 35.33 ms   | 28.55 ms … 154.93 ms          |
| Ws_CAS_TP 1000b / 6t         | 69.63 ms ± 34.91 ms   | 28.55 ms … 154.93 ms          |
| Ws_CAS_TP 10000b / 6t        | 72.22 ms ± 34.75 ms   | 28.55 ms … 154.93 ms          |
| LockingTP 100b / 8t          | 68.83 ms ± 35.65 ms   | 23.95 ms … 154.93 ms          |
| LockingTP 1000b / 8t         | 65.88 ms ± 36.16 ms   | 23.83 ms … 154.93 ms          |
| LockingTP 10000b / 8t        | 63.88 ms ± 35.88 ms   | 23.83 ms … 154.93 ms          |
| Ws_CAS_TP 100b / 8t          | 67.04 ms ± 37.05 ms   | 23.83 ms … 154.93 ms          |
| Ws_CAS_TP 1000b / 8t         | 69.60 ms ± 37.54 ms   | 23.83 ms … 154.93 ms          |
| Ws_CAS_TP 10000b / 8t        | 72.29 ms ± 38.36 ms   | 23.83 ms … 154.93 ms          |
| LockingTP 100b / 16t         | 69.61 ms ± 39.17 ms   | 17.47 ms … 154.93 ms          |
| LockingTP 1000b / 16t        | 67.15 ms ± 39.78 ms   | 17.17 ms … 154.93 ms          |
| LockingTP 10000b / 16t       | 65.01 ms ± 40.08 ms   | 17.17 ms … 154.93 ms          |
| Ws_CAS_TP 100b / 16t         | 70.40 ms ± 47.14 ms   | 17.17 ms … 265.88 ms          |
| Ws_CAS_TP 1000b / 16t        | 74.00 ms ± 49.31 ms   | 17.17 ms … 265.88 ms          |
| Ws_CAS_TP 10000b / 16t       | 77.09 ms ± 50.65 ms   | 17.17 ms … 265.88 ms          |
| LockingTP 100b / 32t         | 74.89 ms ± 50.87 ms   | 17.17 ms … 265.88 ms          |
| LockingTP 1000b / 32t        | 72.80 ms ± 51.04 ms   | 17.17 ms … 265.88 ms          |
| LockingTP 10000b / 32t       | 70.92 ms ± 51.07 ms   | 17.17 ms … 265.88 ms          |
| Ws_CAS_TP 100b / 32t         | 84.63 ms ± 88.65 ms   | 17.17 ms … 543.82 ms          |
| Ws_CAS_TP 1000b / 32t        | 91.66 ms ± 95.12 ms   | 17.17 ms … 543.82 ms          |
| Ws_CAS_TP 10000b / 32t       | 97.83 ms ± 99.52 ms   | 17.17 ms … 543.82 ms          |
| LockingTP 100b / 64t         | 95.39 ms ± 98.90 ms   | 17.17 ms … 543.82 ms          |
| LockingTP 1000b / 64t        | 93.09 ms ± 98.25 ms   | 17.17 ms … 543.82 ms          |
| LockingTP 10000b / 64t       | 91.77 ms ± 97.09 ms   | 17.17 ms … 543.82 ms          |
| Ws_CAS_TP 100b / 64t         | 122.78 ms ± 205.27 ms | 17.17 ms … 1351.03 ms         |
| Ws_CAS_TP 1000b / 64t        | 138.51 ms ± 223.09 ms | 17.17 ms … 1351.03 ms         |
| Ws_CAS_TP 10000b / 64t       | 151.95 ms ± 234.43 ms | 17.17 ms … 1351.03 ms         |



### Summary (Baseline(Fastest):  `LockingTP 10000b / 6t`)

| Configuration                   | Relative Speed (Slower By)     |
|---------------------------------|--------------------------------|
| LockingTP 10000b / 8t           | 1.00 ± 0.79 times slower       |
| LockingTP 10000b / 4t           | 1.01 ± 0.82 times slower       |
| LockingTP 10000b / 16t          | 1.02 ± 0.81 times slower       |
| LockingTP 1000b / 8t            | 1.03 ± 0.75 times slower       |
| LockingTP 1000b / 6t            | 1.05 ± 0.73 times slower       |
| Ws_CAS_TP 100b / 6t             | 1.05 ± 0.73 times slower       |
| Ws_CAS_TP 100b / 8t             | 1.05 ± 0.74 times slower       |
| LockingTP 1000b / 16t           | 1.05 ± 0.77 times slower       |
| LockingTP 100b / 8t             | 1.08 ± 0.70 times slower       |
| Ws_CAS_TP 1000b / 8t            | 1.09 ± 0.71 times slower       |
| LockingTP 100b / 16t            | 1.09 ± 0.72 times slower       |
| Ws_CAS_TP 1000b / 6t            | 1.09 ± 0.68 times slower       |
| LockingTP 1000b / 4t            | 1.10 ± 0.76 times slower       |
| Ws_CAS_TP 100b / 16t            | 1.11 ± 0.79 times slower       |
| Ws_CAS_TP 100b / 4t             | 1.11 ± 0.69 times slower       |
| LockingTP 10000b / 32t          | 1.11 ± 0.82 times slower       |
| LockingTP 100b / 6t             | 1.12 ± 0.66 times slower       |
| Ws_CAS_TP 10000b / 6t           | 1.13 ± 0.65 times slower       |
| Ws_CAS_TP 10000b / 8t           | 1.13 ± 0.68 times slower       |
| LockingTP 1000b / 32t           | 1.14 ± 0.78 times slower       |
| Ws_CAS_TP 1000b / 16t           | 1.16 ± 0.75 times slower       |
| Ws_CAS_TP 1000b / 4t            | 1.17 ± 0.62 times slower       |
| LockingTP 100b / 32t            | 1.18 ± 0.75 times slower       |
| Ws_CAS_TP 10000b / 16t          | 1.21 ± 0.71 times slower       |
| Ws_CAS_TP 10000b / 4t           | 1.22 ± 0.58 times slower       |
| Ws_CAS_TP 100b / 32t            | 1.33 ± 0.89 times slower       |
| LockingTP 100b / 4t             | 1.34 ± 0.57 times slower       |
| Ws_CAS_TP 1000b / 32t           | 1.44 ± 0.82 times slower       |
| LockingTP 10000b / 64t          | 1.44 ± 0.83 times slower       |
| LockingTP 1000b / 64t           | 1.46 ± 0.82 times slower       |
| LockingTP 100b / 64t            | 1.50 ± 0.79 times slower       |
| Ws_CAS_TP 10000b / 32t          | 1.54 ± 0.75 times slower       |
| Ws_CAS_TP 100b / 64t            | 1.93 ± 0.91 times slower       |
| Naive                           | 2.06 ± 0.27 times slower       |
| Ws_CAS_TP 1000b / 64t           | 2.17 ± 0.78 times slower       |
| Ws_CAS_TP 10000b / 64t          | 2.39 ± 0.69 times slower       |


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