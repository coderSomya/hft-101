// compile_time_dispatch_optimized.cpp
// Compile-time dispatch using if constexpr and templates

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <type_traits>

// Generic compute function with compile-time dispatch
template<typename T>
double compute_heavy_operation(const T& value) {
    if constexpr (std::is_integral_v<T>) {
        // Integer-specific logic
        volatile long long result = 1;
        for (int i = 1; i <= 10; ++i) {
            result *= value + i;
        }
        return static_cast<double>(result);
    } else if constexpr (std::is_floating_point_v<T>) {
        // Float-specific logic
        volatile T result = value;
        for (int i = 0; i < 10; ++i) {
            result = result * T(0.987) + T(1.234);
        }
        return static_cast<double>(result);
    } else {
        static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic");
        return 0.0;
    }
}

// Wrapper to handle variant-like behavior at compile time
template<typename... Args>
double process_mixed(Args&&... args) {
    double total = 0.0;
    ((total += compute_heavy_operation(args)), ...); // C++17 fold expr
    return total;
}

const size_t N = 1 << 16; // 65K operations

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> int_dist(1, 100);
    std::uniform_real_distribution<float> float_dist(1.0f, 100.0f);
    std::bernoulli_distribution is_int(0.5);

    // Generate data
    std::vector<int> ints;
    std::vector<float> floats;
    ints.reserve(N/2);
    floats.reserve(N/2);

    for (size_t i = 0; i < N; ++i) {
        if (is_int(gen)) {
            ints.push_back(int_dist(gen));
        } else {
            floats.push_back(float_dist(gen));
        }
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Process all integers
    double total = 0.0;
    for (const auto& val : ints) {
        total += compute_heavy_operation(val); // Inlined, no branch
    }
    for (const auto& val : floats) {
        total += compute_heavy_operation(val); // Inlined, no branch
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile double result = total;
    (void)result;

    std::cout << "Optimized (compile-time dispatch) version latency: " << duration.count() << " microseconds\n";

    return 0;
}