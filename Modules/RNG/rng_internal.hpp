#ifndef RNG_INTERNAL_HPP
# define RNG_INTERNAL_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
#include <atomic>
#include <cstdint>
#include <random>
#include "../PThread/mutex.hpp"

extern std::mt19937 g_random_engine;
extern std::atomic<ft_bool> g_random_engine_seeded;

void ft_seed_random_engine(uint32_t seed_value);
void ft_seed_random_engine_with_entropy(void);
int32_t rng_enable_random_engine_thread_safety(void);
int32_t rng_disable_random_engine_thread_safety(void);
int32_t rng_lock_random_engine_mutex(void);
int32_t rng_unlock_random_engine_mutex(void);

inline __attribute__((always_inline)) void ft_init_random_engine(void)
{
    if (g_random_engine_seeded.load(std::memory_order_acquire) == FT_FALSE)
        ft_seed_random_engine_with_entropy();
    return ;
}

#endif
