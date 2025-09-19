// lock_free_programming_naive.cpp
// Uses mutex-based stack — traditional locking

#include <iostream>
#include <thread>
#include <vector>
#include <stack>
#include <mutex>
#include <chrono>

class LockedStack {
private:
    std::stack<int> stack_;
    std::mutex mtx_;

public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mtx_);
        stack_.push(value);
    }

    bool pop(int& value) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (stack_.empty()) return false;
        value = stack_.top();
        stack_.pop();
        return true;
    }
};

const int NUM_THREADS = 4;
const int OPERATIONS_PER_THREAD = 50000;

void worker(LockedStack& stack, int id) {
    for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
        if (i % 2 == 0) {
            stack.push(i * id);
        } else {
            int val;
            stack.pop(val);
        }
    }
}

int main() {
    LockedStack stack;
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(worker, std::ref(stack), i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Naive (mutex-based) version latency: " << duration.count() << " microseconds\n";

    return 0;
}