#ifndef RNG_INTERNAL_HPP
# define RNG_INTERNAL_HPP

#include <atomic>
#include <cstdint>
#include <random>
#include "../PThread/mutex.hpp"

extern std::mt19937 g_random_engine;
extern pt_mutex g_random_engine_mutex;
extern std::atomic<bool> g_random_engine_seeded;

void ft_seed_random_engine(uint32_t seed_value);
void ft_seed_random_engine_with_entropy(void);

inline __attribute__((always_inline)) void ft_init_random_engine(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == false)
        ft_seed_random_engine_with_entropy();
    return ;
}

#endif
