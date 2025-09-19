# Compile naive version
g++ -std=c++17 -O3 short_circuiting_naive.cpp -o sc_naive

# Compile optimized version
g++ -std=c++17 -O3 short_circuiting_optimized.cpp -o sc_optimized

# Run both
./sc_naive
./sc_optimized