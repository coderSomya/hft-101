# Constexpr: Teaching Guide

## What is `constexpr`?

`constexpr` (short for *constant expression*) is a C++ keyword that indicates a value or function can be evaluated **at compile time**. Introduced in C++11 and significantly enhanced in C++14, C++17, and C++20, `constexpr` enables **compile-time computation**, leading to:

- Zero runtime cost for known values
- Improved performance (no runtime calculation needed)
- Enhanced safety (values validated before program runs)
- Better optimization opportunities (constant folding, dead code elimination)

## Why Use `constexpr`?

- **Performance**: Move computation from runtime to compile time
- **Correctness**: Catch errors early (e.g., invalid array sizes, out-of-bounds access)
- **Generic Programming**: Enable compile-time logic for templates
- **Embedded & Real-Time Systems**: Guarantee deterministic startup with no runtime init cost