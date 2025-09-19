// short_circuiting_optimized.cpp
// Optimized version: Uses logical && with short-circuiting to skip expensive checks

#include <iostream>
#include <vector>
#include <chrono>
#include <random>

const size_t N = 1 << 20; // 1M iterations

// Same expensive function
bool expensive_check(int index, const std::vector<int>& data) {
    volatile long long sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += data[index % data.size()] * i;
    }
    return (sum & 1) == 0;
}

// Optimized: Uses short-circuiting — bounds check FIRST
bool check_access_optimized(size_t index, const std::vector<int>& data) {
    // Short-circuit: if index >= size, expensive_check is NEVER called
    return (index < data.size()) && expensive_check(index, data);
}

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, N); // Generates mostly OOB indices

    auto start = std::chrono::high_resolution_clock::now();

    size_t valid_count = 0;
    for (size_t i = 0; i < N; ++i) {
        size_t index = dist(gen);
        if (check_access_optimized(index, data)) {
            valid_count++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile size_t result = valid_count;
    (void)result;

    std::cout << "Optimized (with short-circuiting) version latency: " << duration.count() << " microseconds\n";

    return 0;
}