#include "rng_internal.hpp"
#include <atomic>
#include <mutex>
#include <random>

std::mt19937 g_random_engine;
std::mutex g_random_engine_mutex;
std::atomic<bool> g_random_engine_seeded(false);

void ft_seed_random_engine(uint32_t seed_value)
{
    std::lock_guard<std::mutex> guard(g_random_engine_mutex);
    g_random_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    g_random_engine_seeded.store(true, std::memory_order_release);
    return ;
}

void ft_seed_random_engine_with_entropy(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == true)
        return ;
    std::random_device random_device;
    uint32_t seed_value;

    seed_value = random_device();
    if (g_random_engine_seeded.load(std::memory_order_acquire) == true)
        return ;
    ft_seed_random_engine(seed_value);
    return ;
}
