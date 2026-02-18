#ifndef RNG_INTERNAL_HPP
# define RNG_INTERNAL_HPP

#include <atomic>
#include <cstdint>
#include <random>
#include "../PThread/mutex.hpp"

extern std::mt19937 g_random_engine;
extern std::atomic<bool> g_random_engine_seeded;

void ft_seed_random_engine(uint32_t seed_value);
void ft_seed_random_engine_with_entropy(void);
int rng_enable_random_engine_thread_safety(void);
int rng_disable_random_engine_thread_safety(void);
int rng_lock_random_engine_mutex(void);
int rng_unlock_random_engine_mutex(void);

inline __attribute__((always_inline)) void ft_init_random_engine(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == false)
        ft_seed_random_engine_with_entropy();
    return ;
}

#endif
