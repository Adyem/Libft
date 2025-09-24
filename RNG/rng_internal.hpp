#ifndef RNG_INTERNAL_HPP
# define RNG_INTERNAL_HPP

#include <atomic>
#include <cstdint>
#include <mutex>
#include <random>

extern std::mt19937 g_random_engine;
extern std::mutex g_random_engine_mutex;
extern std::atomic<bool> g_random_engine_seeded;

void ft_seed_random_engine(uint32_t seed_value);
void ft_seed_random_engine_with_entropy(void);

inline __attribute__((always_inline)) void ft_init_srand(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == false)
        ft_seed_random_engine_with_entropy();
    return ;
}

#endif
