// hotpath_optimization_optimized.cpp
// Optimized hotpath: SoA layout, manual unrolling, prefetching, inlining

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#ifdef __GNUG__
    #include <xmmintrin.h>
#endif

const size_t N = 1 << 18; // 256K entities
const size_t UNROLL = 4;
const size_t PREFETCH_DISTANCE = 16;

// Structure of Arrays (SoA) - better for vectorization and cache
struct EntitySystem {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<bool> active;

    EntitySystem(size_t n) : x(n), y(n), z(n), vx(n), vy(n), vz(n), active(n) {}
};

// Force inline for hot function
inline void update_entity_hot(
    float& x, float& y, float& z,
    float vx, float vy, float vz,
    bool active,
    float dt
) {
    if (__builtin_expect(active, 1)) { // Hint: likely true
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;
    }
}

int main() {
    EntitySystem entities(N);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_pos(-10.0f, 10.0f);
    std::uniform_real_distribution<float> dist_vel(-1.0f, 1.0f);
    std::uniform_int_distribution<int> dist_active(0, 1);

    for (size_t i = 0; i < N; ++i) {
        entities.x[i] = dist_pos(gen);
        entities.y[i] = dist_pos(gen);
        entities.z[i] = dist_pos(gen);
        entities.vx[i] = dist_vel(gen);
        entities.vy[i] = dist_vel(gen);
        entities.vz[i] = dist_vel(gen);
        entities.active[i] = dist_active(gen) != 0;
    }

    float dt = 0.016f;

    auto start = std::chrono::high_resolution_clock::now();

    // Prefetch first few cache lines
    for (size_t i = 0; i < PREFETCH_DISTANCE && i < N; ++i) {
        #ifdef __GNUG__
            __builtin_prefetch(&entities.x[i], 0, 3);
            __builtin_prefetch(&entities.vx[i], 0, 3);
        #endif
    }

    // Manually unrolled loop
    size_t i = 0;
    for (; i + UNROLL <= N; i += UNROLL) {

        // Prefetch ahead
        if (i + PREFETCH_DISTANCE < N) {
            #ifdef __GNUG__
                __builtin_prefetch(&entities.x[i + PREFETCH_DISTANCE], 0, 3);
                __builtin_prefetch(&entities.vx[i + PREFETCH_DISTANCE], 0, 3);
            #endif
        }

        // Unrolled updates
        update_entity_hot(entities.x[i],   entities.y[i],   entities.z[i],   entities.vx[i],   entities.vy[i],   entities.vz[i],   entities.active[i],   dt);
        update_entity_hot(entities.x[i+1], entities.y[i+1], entities.z[i+1], entities.vx[i+1], entities.vy[i+1], entities.vz[i+1], entities.active[i+1], dt);
        update_entity_hot(entities.x[i+2], entities.y[i+2], entities.z[i+2], entities.vx[i+2], entities.vy[i+2], entities.vz[i+2], entities.active[i+2], dt);
        update_entity_hot(entities.x[i+3], entities.y[i+3], entities.z[i+3], entities.vx[i+3], entities.vy[i+3], entities.vz[i+3], entities.active[i+3], dt);
    }

    // Handle remainder
    for (; i < N; ++i) {
        update_entity_hot(entities.x[i], entities.y[i], entities.z[i], entities.vx[i], entities.vy[i], entities.vz[i], entities.active[i], dt);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Prevent optimization
    volatile float result = entities.x[N/2];
    (void)result;

    std::cout << "Optimized (hotpath optimized) version latency: " << duration.count() << " microseconds\n";

    return 0;
}