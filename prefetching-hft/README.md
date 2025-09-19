# Prefetching: Teaching Guide

## What is Prefetching?

**Prefetching** is a performance optimization technique where data is proactively loaded into the CPU cache *before* it is actually needed by the program. This hides memory latency by overlapping data fetch with computation, especially critical in **memory-bound applications**.

Modern CPUs have hardware prefetchers that detect simple access patterns (e.g., sequential reads) and automatically prefetch cache lines. However, for **irregular or complex access patterns**, software prefetching via explicit instructions can yield significant performance gains.

In C++, prefetching is typically done via compiler intrinsics like:
- `_mm_prefetch` (x86/x64)
- `__builtin_prefetch` (GCC/Clang portable intrinsic)

## Why Use Prefetching?

- Reduces **cache miss penalties** (can be 100s of cycles)
- Improves throughput in **data-intensive loops** (e.g., large array traversals, graph processing)
- Especially useful when **memory latency dominates** over computation
- Can be combined with other optimizations (SIMD, loop unrolling)