// inlining_optimized.cpp
// Optimized version: Functions are inlined — no call overhead

#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// Inline accessors — compiler will likely inline them
inline float get_velocity_x(const std::vector<float>& velocities, size_t i) {
    return velocities[i * 3 + 0];
}

inline float get_velocity_y(const std::vector<float>& velocities, size_t i) {
    return velocities[i * 3 + 1];
}

inline float get_velocity_z(const std::vector<float>& velocities, size_t i) {
    return velocities[i * 3 + 2];
}

// Inline computation — enables constant propagation and vectorization
inline float compute_speed(float vx, float vy, float vz) {
    return vx * vx + vy * vy + vz * vz;
}

const size_t N = 1 << 18; // 256K particles

int main() {
    // AoS-like: [vx0, vy0, vz0, vx1, vy1, vz1, ...]
    std::vector<float> velocities(N * 3);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t i = 0; i < N * 3; ++i) {
        velocities[i] = dist(gen);
    }

    auto start = std::chrono::high_resolution_clock::now();

    float total_speed = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        float vx = get_velocity_x(velocities, i);
        float vy = get_velocity_y(velocities, i);
        float vz = get_velocity_z(velocities, i);
        total_speed += compute_speed(vx, vy, vz);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile float result = total_speed;
    (void)result;

    std::cout << "Optimized (with inlining) version latency: " << duration.count() << " microseconds\n";

    return 0;
}