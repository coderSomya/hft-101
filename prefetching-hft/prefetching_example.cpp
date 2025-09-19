// prefetching_optimized.cpp
// Uses explicit software prefetching to reduce cache misses

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#ifdef __GNUG__
    #include <xmmintrin.h> // for _mm_prefetch (x86 SSE)
#endif

const size_t N = 1 << 22; // 4M elements
const size_t STRIDE = 128;
const size_t PREFETCH_DISTANCE = 32; // Prefetch 32 * STRIDE elements ahead

long long sum_array_prefetch(int* data, size_t n, size_t stride) {
    long long sum = 0;

    // Prefetch first few cache lines
    for (size_t i = 0; i < PREFETCH_DISTANCE * stride && i < n; i += stride) {
        #ifdef __GNUG__
            __builtin_prefetch(&data[i], 0, 3); // Read, high temporal locality
        #endif
    }

    // Main loop
    for (size_t i = 0; i < n; i += stride) {
        // Prefetch ahead
        if (i + PREFETCH_DISTANCE * stride < n) {
            #ifdef __GNUG__
                __builtin_prefetch(&data[i + PREFETCH_DISTANCE * stride], 0, 3);
            #endif
        }

        sum += data[i];
    }

    return sum;
}

int main() {
    // Allocate and initialize large array
    std::vector<int> data(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, 100);

    for (size_t i = 0; i < N; ++i) {
        data[i] = dist(gen);
    }

    // Benchmark
    auto start = std::chrono::high_resolution_clock::now();

    long long result = sum_array_prefetch(data.data(), N, STRIDE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile long long dummy = result;
    (void)dummy;

    std::cout << "Optimized (with prefetching) version latency: " << duration.count() << " microseconds\n";

    return 0;
}