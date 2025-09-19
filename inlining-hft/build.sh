# Compile naive version
g++ -std=c++17 -O3 inlining_naive.cpp -o inline_naive

# Compile optimized version
g++ -std=c++17 -O3 inlining_optimized.cpp -o inline_optimized

# Run both
./inline_naive
./inline_optimized