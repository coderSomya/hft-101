// branch_reduction_optimized.cpp
// Branchless absolute value using bit manipulation

#include <iostream>
#include <vector>
#include <chrono>
#include <random>

const size_t N = 1 << 20; // 1M elements

void compute_abs_branchless(int* input, int* output, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        int x = input[i];
        // Branchless absolute value:
        // mask = (x >> 31) → -1 if x<0, 0 if x>=0
        // (x + mask) ^ mask → if x<0: (x-1)^(-1) = -x; if x>=0: x^0 = x
        int mask = x >> 31;
        output[i] = (x + mask) ^ mask;
    }
}

int main() {
    // Allocate and initialize arrays
    std::vector<int> input(N), output(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(-1000000, 1000000);

    for (size_t i = 0; i < N; ++i) {
        input[i] = dist(gen);
    }

    // Benchmark
    auto start = std::chrono::high_resolution_clock::now();

    compute_abs_branchless(input.data(), output.data(), N);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile int result = output[N/2];
    (void)result;

    std::cout << "Optimized (branchless) version latency: " << duration.count() << " microseconds\n";

    return 0;
}