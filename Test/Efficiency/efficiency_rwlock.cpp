#include "../../PThread/pthread.hpp"
#include "utils.hpp"

#include <cstddef>

int test_efficiency_rwlock_reader_priority(void)
{
    const size_t iterations = 50000;
    pthread_rwlock_t native_lock;
    t_pt_rwlock custom_lock;
    volatile size_t counter;
    size_t index;

    if (pt_rwlock_init(&native_lock, ft_nullptr) != 0)
        return (0);
    if (pt_rwlock_strategy_init(&custom_lock, PT_RWLOCK_STRATEGY_READER_PRIORITY) != ER_SUCCESS)
    {
        pt_rwlock_destroy(&native_lock);
        return (0);
    }
    counter = 0;

    auto start_std = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        pt_rwlock_rdlock(&native_lock);
        counter += index;
        pt_rwlock_unlock(&native_lock);
        index++;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        pt_rwlock_strategy_rdlock(&custom_lock);
        counter += index;
        pt_rwlock_strategy_unlock(&custom_lock);
        index++;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&counter);
    print_comparison("rwlock reader priority rdlock/unlock",
        elapsed_us(start_std, end_std), elapsed_us(start_ft, end_ft));

    pt_rwlock_destroy(&native_lock);
    pt_rwlock_strategy_destroy(&custom_lock);
    return (1);
}

int test_efficiency_rwlock_writer_priority(void)
{
    const size_t iterations = 50000;
    pthread_rwlock_t native_lock;
    t_pt_rwlock custom_lock;
    volatile size_t counter;
    size_t index;

    if (pt_rwlock_init(&native_lock, ft_nullptr) != 0)
        return (0);
    if (pt_rwlock_strategy_init(&custom_lock, PT_RWLOCK_STRATEGY_WRITER_PRIORITY) != ER_SUCCESS)
    {
        pt_rwlock_destroy(&native_lock);
        return (0);
    }
    counter = 0;

    auto start_std = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        pt_rwlock_wrlock(&native_lock);
        counter += index;
        pt_rwlock_unlock(&native_lock);
        index++;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        pt_rwlock_strategy_wrlock(&custom_lock);
        counter += index;
        pt_rwlock_strategy_unlock(&custom_lock);
        index++;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&counter);
    print_comparison("rwlock writer priority wrlock/unlock",
        elapsed_us(start_std, end_std), elapsed_us(start_ft, end_ft));

    pt_rwlock_destroy(&native_lock);
    pt_rwlock_strategy_destroy(&custom_lock);
    return (1);
}
