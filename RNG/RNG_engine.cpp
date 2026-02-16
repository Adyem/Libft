#include "rng_internal.hpp"
#include <atomic>
#include <random>

std::mt19937 g_random_engine;
pt_mutex g_random_engine_mutex;
std::atomic<bool> g_random_engine_seeded(false);

void ft_seed_random_engine(uint32_t seed_value)
{
    int lock_error;
    int unlock_error;

    lock_error = g_random_engine_mutex.lock();
    if (lock_error != 0)
        return ;
    g_random_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    g_random_engine_seeded.store(true, std::memory_order_release);
    unlock_error = g_random_engine_mutex.unlock();
    if (unlock_error != 0)
        return ;
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
