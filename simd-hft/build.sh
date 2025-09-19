# Compile naive version
g++ -std=c++17 -O3 simd_naive.cpp -o simd_naive

# Compile optimized (SIMD) version
g++ -std=c++17 -O3 -mavx2 simd_optimized.cpp -o simd_optimized

# Run both
./simd_naive
./simd_optimized