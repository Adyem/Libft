#include "geometry_lock_tracker.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <chrono>
#include <functional>
#include <new>
#include <random>
#include <vector>

struct s_geometry_wait_record
{
    pt_thread_id_type thread_identifier;
    const void *owned_object;
    const void *requested_object;
};

static pt_mutex g_geometry_tracker_mutex;
static std::vector<s_geometry_wait_record> g_geometry_wait_records;

static bool geometry_lock_tracker_register_wait(pt_thread_id_type thread_identifier,
        const void *owned_object, const void *requested_object,
        bool &out_cycle_detected)
{
    int lock_error;

    lock_error = pt_mutex_lock_with_error(g_geometry_tracker_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    ft_global_error_stack_drop_last_error();
    size_t index;
    bool found;

    index = 0;
    found = false;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier == thread_identifier)
        {
            g_geometry_wait_records[index].owned_object = owned_object;
            g_geometry_wait_records[index].requested_object = requested_object;
            found = true;
        }
        index += 1;
    }
    if (!found)
    {
        s_geometry_wait_record record;

        record.thread_identifier = thread_identifier;
        record.owned_object = owned_object;
        record.requested_object = requested_object;
        try
        {
            g_geometry_wait_records.push_back(record);
        }
        catch (const std::bad_alloc &)
        {
            int unlock_error;

            unlock_error = pt_mutex_unlock_with_error(g_geometry_tracker_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                ft_global_error_stack_push(unlock_error);
                return (false);
            }
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (false);
        }
    }
    bool cycle_detected;

    cycle_detected = false;
    index = 0;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier != thread_identifier)
        {
            if (g_geometry_wait_records[index].owned_object == requested_object
                && g_geometry_wait_records[index].requested_object == owned_object)
                cycle_detected = true;
        }
        index += 1;
    }
    out_cycle_detected = cycle_detected;
    int unlock_error;

    unlock_error = pt_mutex_unlock_with_error(g_geometry_tracker_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (false);
    ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (true);
}

static void geometry_lock_tracker_clear_wait(pt_thread_id_type thread_identifier)
{
    int lock_error;

    lock_error = pt_mutex_lock_with_error(g_geometry_tracker_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    ft_global_error_stack_drop_last_error();
    size_t index;

    index = 0;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier == thread_identifier)
        {
            size_t shift_index;

            shift_index = index;
            while (shift_index + 1 < g_geometry_wait_records.size())
            {
                g_geometry_wait_records[shift_index] = g_geometry_wait_records[shift_index + 1];
                shift_index += 1;
            }
            if (!g_geometry_wait_records.empty())
                g_geometry_wait_records.pop_back();
            break;
        }
        index += 1;
    }
    int unlock_error;

    unlock_error = pt_mutex_unlock_with_error(g_geometry_tracker_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

static void geometry_lock_tracker_sleep_backoff()
{
    static thread_local bool generator_initialized = false;
    static thread_local std::minstd_rand generator;
    std::uniform_int_distribution<int> distribution(1, 10);
    unsigned long long time_seed;
    std::size_t address_seed;
    unsigned int combined_seed;
    int delay_ms;

    if (!generator_initialized)
    {
        time_seed = static_cast<unsigned long long>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        address_seed = reinterpret_cast<std::size_t>(&generator);
        combined_seed = static_cast<unsigned int>(time_seed ^ address_seed);
        if (combined_seed == 0)
            combined_seed = static_cast<unsigned int>(address_seed | 1U);
        generator.seed(combined_seed);
        generator_initialized = true;
    }
    delay_ms = distribution(generator);
    pt_thread_sleep(static_cast<unsigned int>(delay_ms));
    return ;
}

int geometry_lock_tracker_lock_pair(const void *first_object, const void *second_object,
        pt_recursive_mutex &first_mutex, pt_recursive_mutex &second_mutex)
{
    if (first_object == second_object)
    {
        int self_error;

        self_error = pt_recursive_mutex_lock_with_error(first_mutex);
        if (self_error != FT_ERR_SUCCESS)
            return (self_error);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const void *ordered_first_object;
    const void *ordered_second_object;
    pt_recursive_mutex *ordered_first_mutex;
    pt_recursive_mutex *ordered_second_mutex;
    std::less<const void *> pointer_less;

    ordered_first_object = first_object;
    ordered_second_object = second_object;
    ordered_first_mutex = &first_mutex;
    ordered_second_mutex = &second_mutex;
    if (pointer_less(ordered_second_object, ordered_first_object))
    {
        const void *temporary_object;
        pt_recursive_mutex *temporary_mutex;

        temporary_object = ordered_first_object;
        ordered_first_object = ordered_second_object;
        ordered_second_object = temporary_object;
        temporary_mutex = ordered_first_mutex;
        ordered_first_mutex = ordered_second_mutex;
        ordered_second_mutex = temporary_mutex;
    }
    while (true)
    {
        int lower_error;

        lower_error = pt_recursive_mutex_lock_with_error(*ordered_first_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        ft_global_error_stack_drop_last_error();
        bool cycle_detected;

        cycle_detected = false;
        if (!geometry_lock_tracker_register_wait(THREAD_ID, ordered_first_object,
                ordered_second_object, cycle_detected))
        {
            int wait_error;
            int unlock_error;

            wait_error = ft_global_error_stack_drop_last_error();
            unlock_error = pt_recursive_mutex_unlock_with_error(*ordered_first_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
                return (unlock_error);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(wait_error);
            return (wait_error);
        }
        int upper_error;

        upper_error = pt_recursive_mutex_lock_with_error(*ordered_second_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            ft_global_error_stack_drop_last_error();
        (void)cycle_detected;
        geometry_lock_tracker_clear_wait(THREAD_ID);
        if (upper_error == FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            int unlock_error;

            unlock_error = pt_recursive_mutex_unlock_with_error(*ordered_first_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
                return (unlock_error);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(upper_error);
            return (upper_error);
        }
        int unlock_error;

        unlock_error = pt_recursive_mutex_unlock_with_error(*ordered_first_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        ft_global_error_stack_drop_last_error();
        geometry_lock_tracker_sleep_backoff();
    }
}
