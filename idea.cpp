#include <bits/stdc++.h>
#include <tuple>

// template <auto Fn>
// class ThreadPool;
//
// void work(int n, int &result) {}
// void job(int n, int &result) {}
//
using namespace std;

void inc(int a, double k, int &c) { c += a; }

int main() {

  // WorkerManager<8> workers;
  //
  // ThreadPool<work, workers> work_pool;
  //
  // int k[4];
  // work_pool.submit(3, k[0]);
  // work_pool.submit(1, k[1]);
  // work_pool.submit(8, k[2]);
  // work_pool.submit(9, k[3]);
  //
  // work_pool.wait();
  //
  // ThreadPool<job, workers> job_pool = work_pool.exchange(
  //

  int k = 9;
  tuple<int, double, int &> b(12, 0.2, k);

  std::apply(inc, std::move(b));

  std::cout << k;

  return 0;
}
