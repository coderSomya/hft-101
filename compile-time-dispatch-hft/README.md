# Compile-Time Dispatch: Teaching Guide

## What is Compile-Time Dispatch?

**Compile-time dispatch** is a C++ technique that selects the appropriate function, algorithm, or code path **at compile time** — based on types, values, or traits — rather than using runtime conditionals (`if`, `switch`, virtual calls). This eliminates runtime overhead and enables deeper compiler optimizations.

Common mechanisms:
- **Function overloading**
- **Template specialization**
- **Tag dispatching**
- **`if constexpr` (C++17)**
- **Concepts (C++20)**

## Why Use Compile-Time Dispatch?

- **Zero runtime cost** — no branches, no vtable lookups
- **Enables aggressive optimization** — compiler sees only the selected path
- **Type safety** — invalid combinations caught at compile time
- **Performance critical systems** — games, HPC, embedded, low-latency trading
