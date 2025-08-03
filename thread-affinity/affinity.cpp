#include <thread>
#include <benchmark/benchmark.h>
#include <pthread.h>
#include <atomic>
#include <cassert>

struct AlignedAtomic{
    alignas(64) std::atomic<int> val=0;
};

void thread_affinity(){

    AlignedAtomic a;
    AlignedAtomic b;

    cpu_set_t cpu_set_0;
    cpu_set_t cpu_set_1;

    // Zero them out
    CPU_ZERO(0,cpu_set_0);
    CPU_ZERO(1,cpu_set_1);

    CPU_SET(0,&cpu_set_0);
    CPU_SET(1,&cpu_set_1);

    auto work = [](AlignedAtomic &atomic){
        for(int i=0; i<(1<<20); i++) atomic.val++;
    };

    std::thread t0(work, std::ref(a));
    std::thread t1(work, std::ref(b));

    assert(pthread_setaffinity_np(t0.native_handle(), sizeof(cpu_set_t),
                                  &cpu_set_0) == 0);
    assert(pthread_setaffinity_np(t1.native_handle(), sizeof(cpu_set_t),
                                  &cpu_set_0) == 0);

    std::thread t2(work, std::ref(a));
    std::thread t3(work, std::ref(b));

    assert(pthread_setaffinity_np(t2.native_handle(), sizeof(cpu_set_t),
                                  &cpu_set_1) == 0);
    assert(pthread_setaffinity_np(t3.native_handle(), sizeof(cpu_set_t),
                                  &cpu_set_1) == 0);

    t0.join();
    t1.join();
    t2.join();
    t3.join();

}

static void thread_affinity(benchmark::State &s){
    for (auto _ : s) thread_affinity();
}

BENCHMARK(thread_affinity)->UseRealTime()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
