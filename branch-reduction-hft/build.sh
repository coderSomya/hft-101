# Compile naive version
g++ -std=c++17 -O3 branch_reduction_naive.cpp -o branch_naive

# Compile optimized version
g++ -std=c++17 -O3 branch_reduction_optimized.cpp -o branch_optimized

# Run both
./branch_naive
./branch_optimized