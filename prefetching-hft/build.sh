# Compile naive version
g++ -std=c++17 -O3 prefetching_naive.cpp -o prefetch_naive

# Compile optimized version
g++ -std=c++17 -O3 prefetching_optimized.cpp -o prefetch_optimized

# Run both
./prefetch_naive
./prefetch_optimized