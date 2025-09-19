# Inlining: Teaching Guide

## What is Inlining?

**Inlining** is a compiler optimization that replaces a function call with the actual body of the function at the call site. This eliminates the overhead of:
- Function call/return instructions (`call`/`ret`)
- Stack frame setup/teardown
- Parameter passing and return value handling

It also enables **further optimizations** like constant propagation, dead code elimination, and vectorization — because the compiler can now “see through” the function boundary.

In C++, you request inlining with the `inline` keyword, but it’s only a *hint*. The compiler ultimately decides based on heuristics (size, call frequency, optimization level).

## Why Use Inlining?

- **Reduces function call overhead** — critical in tight loops
- **Enables interprocedural optimizations** — compiler optimizes across function boundaries
- **Improves instruction cache locality** — code is contiguous
- **Facilitates constant folding** — if arguments are known at compile time
