// cache_warming_optimized.cpp
// Uses explicit cache warming — touch data before timing-critical section

#include <iostream>
#include <vector>
#include <chrono>
#include <random>

const size_t N = 1 << 22; // 4M elements
const size_t CACHE_LINE_SIZE = 64;
const size_t STRIDE = CACHE_LINE_SIZE / sizeof(int); // 16 ints per 64B line

void warm_cache(int* data, size_t n, size_t stride) {
    // Touch one element per cache line — triggers cache load
    for (size_t i = 0; i < n; i += stride) {
        volatile int tmp = data[i]; // 'volatile' prevents optimization
        (void)tmp;
    }
}

long long sum_array_optimized(int* data, size_t n, size_t stride) {
    long long sum = 0;
    for (size_t i = 0; i < n; i += stride) {
        sum += data[i]; // Now likely in L1/L2 cache
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

    // WARM CACHE BEFORE TIMING
    warm_cache(data.data(), N, STRIDE);

    // Now benchmark — cache is warm
    auto start = std::chrono::high_resolution_clock::now();

    long long result = sum_array_optimized(data.data(), N, STRIDE);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile long long dummy = result;
    (void)dummy;

    std::cout << "Optimized (with cache warming) version latency: " << duration.count() << " microseconds\n";

    return 0;
}