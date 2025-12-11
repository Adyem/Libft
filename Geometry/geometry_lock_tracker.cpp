#include "geometry_lock_tracker.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

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
    ft_unique_lock<pt_mutex> guard(g_geometry_tracker_mutex);

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (false);
    }
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
            if (guard.owns_lock())
            {
                guard.unlock();
                if (guard.get_error() != FT_ERR_SUCCESSS)
                {
                    ft_errno = guard.get_error();
                    return (false);
                }
            }
            ft_errno = FT_ERR_NO_MEMORY;
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
    if (guard.owns_lock())
    {
        guard.unlock();
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = guard.get_error();
            return (false);
        }
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

static void geometry_lock_tracker_clear_wait(pt_thread_id_type thread_identifier)
{
    ft_unique_lock<pt_mutex> guard(g_geometry_tracker_mutex);

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
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
    if (guard.owns_lock())
    {
        guard.unlock();
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = guard.get_error();
            return ;
        }
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

static void geometry_lock_tracker_sleep_backoff(bool cycle_detected)
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
    if (cycle_detected)
        ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    else
        ft_errno = FT_ERR_SUCCESSS;
    pt_thread_sleep(static_cast<unsigned int>(delay_ms));
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int geometry_lock_tracker_lock_pair(const void *first_object, const void *second_object,
        pt_mutex &first_mutex, pt_mutex &second_mutex,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    if (first_object == second_object)
    {
        ft_unique_lock<pt_mutex> single_guard(first_mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    const void *ordered_first_object;
    const void *ordered_second_object;
    pt_mutex *ordered_first_mutex;
    pt_mutex *ordered_second_mutex;
    bool swapped;
    std::less<const void *> pointer_less;

    ordered_first_object = first_object;
    ordered_second_object = second_object;
    ordered_first_mutex = &first_mutex;
    ordered_second_mutex = &second_mutex;
    swapped = false;
    if (pointer_less(ordered_second_object, ordered_first_object))
    {
        const void *temporary_object;
        pt_mutex *temporary_mutex;

        temporary_object = ordered_first_object;
        ordered_first_object = ordered_second_object;
        ordered_second_object = temporary_object;
        temporary_mutex = ordered_first_mutex;
        ordered_first_mutex = ordered_second_mutex;
        ordered_second_mutex = temporary_mutex;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(*ordered_first_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        bool cycle_detected;

        cycle_detected = false;
        if (!geometry_lock_tracker_register_wait(THREAD_ID, ordered_first_object,
                ordered_second_object, cycle_detected))
        {
            if (lower_guard.owns_lock())
            {
                lower_guard.unlock();
                if (lower_guard.get_error() != FT_ERR_SUCCESSS)
                    return (lower_guard.get_error());
            }
            return (ft_errno);
        }
        ft_unique_lock<pt_mutex> upper_guard(*ordered_second_mutex);

        geometry_lock_tracker_clear_wait(THREAD_ID);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            if (lower_guard.owns_lock())
            {
                lower_guard.unlock();
                if (lower_guard.get_error() != FT_ERR_SUCCESSS)
                    return (lower_guard.get_error());
            }
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
        {
            lower_guard.unlock();
            if (lower_guard.get_error() != FT_ERR_SUCCESSS)
            {
                ft_errno = lower_guard.get_error();
                return (lower_guard.get_error());
            }
        }
        geometry_lock_tracker_sleep_backoff(cycle_detected);
    }
}
