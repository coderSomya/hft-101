# Cache Warming: Teaching Guide

## What is Cache Warming?

**Cache warming** (also called *cache preloading* or *cache priming*) is a performance optimization technique where data or code is intentionally accessed **before it’s actually needed**, so that it resides in the CPU’s faster cache levels (L1, L2, sometimes L3) when the real computation begins. This hides memory latency and reduces stalls during critical execution paths.

It’s closely related to — but distinct from — **prefetching**:
- Prefetching = hint to hardware to load specific addresses
- Cache warming = manually touch data to trigger cache load

## Why Warm the Cache?

- **Reduce first-access latency** — avoid cold misses at critical moments
- **Improve responsiveness** in real-time systems (games, trading, UI)
- **Stabilize benchmark performance** — avoid noise from cold cache effects
- **Prepare for bursty workloads** — load data during idle periods