// slowpath_removal_optimized.cpp
// Optimized version: slowpath (error handling) moved to separate non-inlined function

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <stdexcept>

const size_t N = 1 << 20; // 1M iterations

// Separate slowpath function — compiler won't inline this
[[noreturn]] __attribute__((noinline))
void handle_allocation_failure(size_t iteration) {
    // Simulate logging, stack trace, or fallback — expensive operations
    std::cerr << "Allocation failed at iteration " << iteration << "\n";
    // In real code: throw, abort, or trigger fallback allocator
    // Here we just return to continue benchmark
}

// Hot path: allocation success (99.999% of the time)
void* allocate_memory_optimized(size_t size, bool& success) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dist(1, 100000);

    if (__builtin_expect(dist(gen) == 1, 0)) { // Hint: unlikely
        success = false;
        return nullptr;
    }

    success = true;
    return new char[size];
}

int main() {
    std::vector<void*> allocations;
    allocations.reserve(N);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < N; ++i) {
        bool success;
        void* ptr = allocate_memory_optimized(64, success);

        if (!success) {
            // SLOWPATH REMOVED FROM HOT LOOP
            handle_allocation_failure(i); // Call isolated slowpath
            continue;
        }

        allocations.push_back(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile size_t result = allocations.size();
    (void)result;

    std::cout << "Optimized (slowpath removed) version latency: " << duration.count() << " microseconds\n";

    // Cleanup
    for (void* ptr : allocations) {
        delete[] static_cast<char*>(ptr);
    }

    return 0;
}