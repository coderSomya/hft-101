# Compile naive version
g++ -std=c++17 -O3 cache_warming_naive.cpp -o cache_naive

# Compile optimized version
g++ -std=c++17 -O3 cache_warming_optimized.cpp -o cache_optimized

# Run both
./cache_naive
./cache_optimized