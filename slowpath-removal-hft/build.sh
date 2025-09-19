# Compile naive version
g++ -std=c++17 -O3 slowpath_removal_naive.cpp -o slowpath_naive

# Compile optimized version
g++ -std=c++17 -O3 slowpath_removal_optimized.cpp -o slowpath_optimized

# Run both
./slowpath_naive
./slowpath_optimized