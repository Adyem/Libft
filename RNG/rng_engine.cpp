#include "rng_internal.hpp"
#include <atomic>
#include <random>
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"

std::mt19937 g_random_engine;
pt_mutex g_random_engine_mutex;
std::atomic<bool> g_random_engine_seeded(false);

void ft_seed_random_engine(uint32_t seed_value)
{
    ft_unique_lock<pt_mutex> guard(g_random_engine_mutex);
    if (guard.get_error() != ER_SUCCESS)
        return ;
    g_random_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    g_random_engine_seeded.store(true, std::memory_order_release);
    ft_errno = ER_SUCCESS;
    return ;
}

void ft_seed_random_engine_with_entropy(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == true)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    std::random_device random_device;
    uint32_t seed_value;

    seed_value = random_device();
    if (g_random_engine_seeded.load(std::memory_order_acquire) == true)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    ft_seed_random_engine(seed_value);
    return ;
}
