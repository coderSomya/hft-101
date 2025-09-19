# SIMD Programming: Teaching Guide

## What is SIMD?

**Single Instruction, Multiple Data (SIMD)** is a class of parallel computing instructions that allow a processor to perform the same operation on multiple data points simultaneously. This exploits **data-level parallelism** and is especially effective in numerical computing, image processing, machine learning, physics simulations, and more.

Modern CPUs support SIMD through extensions like:
- **x86/x64**: SSE, AVX, AVX2, AVX-512
- **ARM**: NEON, SVE
- **RISC-V**: V extension

In C++, you can access SIMD via:
- Compiler intrinsics (`<immintrin.h>`, `<arm_neon.h>`)
- Auto-vectorization (compiler does it for you)
- Libraries like `std::experimental::simd` (C++20 TS), Eigen, Vc

## Why Use SIMD?

- **4x–16x speedup** on suitable workloads (e.g., adding 8 floats at once with AVX2)
- Efficient use of wide CPU registers (128-bit, 256-bit, 512-bit)
- Reduces instruction count and improves throughput