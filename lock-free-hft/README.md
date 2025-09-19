# Lock-Free Programming: Teaching Guide

## What is Lock-Free Programming?

Lock-free programming is a concurrent programming paradigm where multiple threads operate on shared data without using traditional mutual exclusion mechanisms like mutexes or semaphores. Instead, it relies on atomic operations and memory ordering guarantees provided by the hardware and language runtime (e.g., C++ `std::atomic`).

A system is considered *lock-free* if at least one thread is guaranteed to make progress in a finite number of steps — even if other threads are delayed or suspended.

## Why Use Lock-Free?

- **Avoids deadlocks and priority inversion**
- **Reduces thread blocking** → better scalability under contention
- **Lower latency** in high-contention scenarios
- **Better utilization** of multi-core systems

## How Does It Work?

Lock-free algorithms typically use:

- **Atomic Read-Modify-Write (RMW)** operations: `compare_exchange_weak`, `fetch_add`, etc.
- **Memory ordering constraints**: `memory_order_relaxed`, `memory_order_acquire`, etc.
- **Retry loops (optimistic concurrency)**: Attempt operation, retry if failed due to concurrent modification.

## Example: Lock-Free Stack

A simple lock-free stack uses `compare_exchange_weak` to atomically update the head pointer.

## Pitfalls

- **ABA Problem**: A pointer is reused after being freed, causing `compare_exchange` to succeed incorrectly.
  → Mitigation: Use tagged pointers or hazard pointers.
- **Memory Reclamation**: When to safely delete nodes? Use techniques like epoch-based reclamation or hazard pointers.
- **Complexity**: Harder to reason about than lock-based code.

## Best Practices

- Prefer `compare_exchange_weak` in loops (faster on some architectures).
- Use appropriate memory ordering — don’t default to `seq_cst` unless necessary.
- Test under high contention.
- Consider using established libraries (e.g., `folly`, `boost.lockfree`) before rolling your own.

## When NOT to Use

- Low-contention scenarios — locks may be simpler and faster.
- Complex data structures — lock-free trees, hash tables are notoriously difficult.
- When correctness is more important than performance.

## Further Reading

- [Herb Sutter - Lock-Free Programming](https://herbsutter.com/2010/09/24/lock-free-programming/)
- C++ Concurrency in Action (Anthony Williams)
- "The Art of Multiprocessor Programming" (Herlihy & Shavit)