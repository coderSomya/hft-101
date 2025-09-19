// lock_free_programming_optimized.cpp
// Uses lock-free stack with atomic compare_exchange

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

struct Node {
    int data;
    Node* next;
    Node(int d) : data(d), next(nullptr) {}
};

class LockFreeStack {
private:
    std::atomic<Node*> head_{nullptr};

public:
    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head_.load(std::memory_order_relaxed);
        do {
            new_node->next = old_head;
        } while (!head_.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_relaxed));
    }

    bool pop(int& value) {
        Node* old_head = head_.load(std::memory_order_relaxed);
        while (old_head != nullptr) {
            if (head_.compare_exchange_weak(old_head, old_head->next, std::memory_order_acquire, std::memory_order_relaxed)) {
                value = old_head->data;
                delete old_head;
                return true;
            }
        }
        return false; // stack empty
    }

    ~LockFreeStack() {
        int val;
        while (pop(val)) {} // drain and delete all nodes
    }
};

const int NUM_THREADS = 4;
const int OPERATIONS_PER_THREAD = 50000;

void worker(LockFreeStack& stack, int id) {
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
    LockFreeStack stack;
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

    std::cout << "Optimized (lock-free) version latency: " << duration.count() << " microseconds\n";

    return 0;
}