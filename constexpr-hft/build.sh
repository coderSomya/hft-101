# Compile naive version
g++ -std=c++17 -O3 constexpr_naive.cpp -o constexpr_naive

# Compile optimized version
g++ -std=c++17 -O3 constexpr_optimized.cpp -o constexpr_optimized

# Run both
./constexpr_naive
./constexpr_optimized