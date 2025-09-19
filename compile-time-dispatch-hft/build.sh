# Compile naive version
g++ -std=c++17 -O3 compile_time_dispatch_naive.cpp -o dispatch_naive

# Compile optimized version
g++ -std=c++17 -O3 compile_time_dispatch_optimized.cpp -o dispatch_optimized

# Run both
./dispatch_naive
./dispatch_optimized