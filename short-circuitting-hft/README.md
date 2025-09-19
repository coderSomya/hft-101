# Short-Circuiting: Teaching Guide

## What is Short-Circuiting?

**Short-circuiting** is a behavior in logical expressions (`&&`, `||`) where evaluation stops as soon as the final result is determined. This is not just a convenience — it’s a **powerful optimization and safety mechanism** built into most programming languages, including C++.

- For `A && B`: if `A` is `false`, `B` is not evaluated (since `false && anything == false`)
- For `A || B`: if `A` is `true`, `B` is not evaluated (since `true || anything == true`)

## Why Use Short-Circuiting?

- **Performance**: Avoid unnecessary computation — especially expensive function calls or memory accesses
- **Safety**: Prevent errors like null pointer dereference or out-of-bounds access
- **Control Flow**: Use as lightweight “if” conditions without braces