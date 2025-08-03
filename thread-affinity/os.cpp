#include <thread>
#include <benchmark/benchmark.h>
#include <pthread.h>
#include <atomic>
#include <cassert>

struct AlignedAtomic{
    alignas(64) std::atomic<int> val=0;
};

void os_scheduler(){
    AlignedAtomic a;
    AlignedAtomic b;

    auto work = [](AlignedAtomic &atomic){
        for (int i=0; i<(1<<12); i++) atomic.val++;
    };

    std::thread t0(work, std::ref(a));
    std::thread t1(work, std::ref(a));
    std::thread t2(work, std::ref(b));
    std::thread t3(work, std::ref(b));


    t0.join();
    t1.join();
    t2.join();
    t3.join();
}

static void os_scheduling(benchmark::State& s) {
  while (s.KeepRunning()) {
    os_scheduler();
  }
}

BENCHMARK(os_scheduling)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
