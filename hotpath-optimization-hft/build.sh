# Compile naive version
g++ -std=c++17 -O3 hotpath_optimization_naive.cpp -o hotpath_naive

# Compile optimized version
g++ -std=c++17 -O3 hotpath_optimization_optimized.cpp -o hotpath_optimized

# Run both
./hotpath_naive
./hotpath_optimized