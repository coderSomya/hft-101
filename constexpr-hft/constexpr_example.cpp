// constexpr_optimized.cpp
// Compile-time computation using constexpr

#include <iostream>
#include <array>
#include <chrono>
#include <cmath>

const size_t TABLE_SIZE = 1000;

// C++14: constexpr functions can have loops and local vars
constexpr std::array<float, TABLE_SIZE> create_sine_table_compiletime() {
    std::array<float, TABLE_SIZE> table = {};
    for (size_t i = 0; i < TABLE_SIZE; ++i) {
        float angle = 2.0f * 3.1415926535f * static_cast<float>(i) / static_cast<float>(TABLE_SIZE);
        // Note: std::sin is NOT constexpr until C++23
        // So we approximate using Taylor series for demonstration
        float sin_approx = angle - (angle * angle * angle) / 6.0f + (angle * angle * angle * angle * angle) / 120.0f;
        table[i] = sin_approx;
    }
    return table;
}

// Force compile-time evaluation
constexpr auto sine_table = create_sine_table_compiletime();

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    // Zero runtime cost — table is embedded in binary
    volatile float result = sine_table[TABLE_SIZE / 2]; // Just to use it
    (void)result;

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Optimized (constexpr compile-time) version latency: " << duration.count() << " microseconds\n";

    return 0;
}