// simd_optimized.cpp
// SIMD (AVX2) implementation of vector addition
// Requires x86_64 with AVX2 support

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <immintrin.h> // AVX2 intrinsics

const size_t N = 1 << 20; // 1M elements
const size_t ALIGNMENT = 32; // 32-byte alignment for AVX2

void add_arrays_simd(float* a, float* b, float* c, size_t n) {
    size_t i = 0;

    // Process 8 elements at a time (256-bit = 8 floats)
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);   // Load 8 floats
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);   // Add them
        _mm256_store_ps(&c[i], vc);          // Store result
    }

    // Handle remainder with scalar loop
    for (; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

int main() {
    // Allocate aligned memory
    float* a = (float*)_mm_malloc(N * sizeof(float), ALIGNMENT);
    float* b = (float*)_mm_malloc(N * sizeof(float), ALIGNMENT);
    float* c = (float*)_mm_malloc(N * sizeof(float), ALIGNMENT);

    if (!a || !b || !c) {
        std::cerr << "Memory allocation failed!\n";
        return 1;
    }

    // Initialize with random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (size_t i = 0; i < N; ++i) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }

    // Benchmark
    auto start = std::chrono::high_resolution_clock::now();

    add_arrays_simd(a, b, c, N);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile float result = c[N/2];
    (void)result;

    std::cout << "Optimized (SIMD AVX2) version latency: " << duration.count() << " microseconds\n";

    // Cleanup
    _mm_free(a);
    _mm_free(b);
    _mm_free(c);

    return 0;
}