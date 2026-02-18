#include "rng_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <atomic>
#include <new>
#include <random>

std::mt19937 g_random_engine;
std::atomic<bool> g_random_engine_seeded(false);

static pt_mutex *g_random_engine_mutex = ft_nullptr;

pt_mutex *rng_random_engine_mutex_instance(void)
{
    return (g_random_engine_mutex);
}

int rng_enable_random_engine_thread_safety(void)
{
    pt_mutex *mutex;

    if (g_random_engine_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex = new (std::nothrow) pt_mutex();
    if (mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int initialize_error = mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex;
        return (initialize_error);
    }
    g_random_engine_mutex = mutex;
    return (FT_ERR_SUCCESS);
}

int rng_disable_random_engine_thread_safety(void)
{
    if (g_random_engine_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = g_random_engine_mutex->destroy();
    delete g_random_engine_mutex;
    g_random_engine_mutex = ft_nullptr;
    return (destroy_error);
}

int rng_lock_random_engine_mutex(void)
{
    pt_mutex *mutex = g_random_engine_mutex;

    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex->lock());
}

int rng_unlock_random_engine_mutex(void)
{
    pt_mutex *mutex = g_random_engine_mutex;

    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex->unlock());
}

void ft_seed_random_engine(uint32_t seed_value)
{
    int lock_error;
    int unlock_error;

    lock_error = rng_lock_random_engine_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    g_random_engine.seed(static_cast<std::mt19937::result_type>(seed_value));
    g_random_engine_seeded.store(true, std::memory_order_release);
    unlock_error = rng_unlock_random_engine_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
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
