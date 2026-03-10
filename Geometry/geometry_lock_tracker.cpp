#include "geometry_lock_tracker.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include <chrono>
#include <functional>
#include <mutex>
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
static std::once_flag g_geometry_tracker_init_once;
static uint32_t g_geometry_tracker_init_error = FT_ERR_SUCCESS;

static void geometry_lock_tracker_initialize_mutex_once()
{
    g_geometry_tracker_init_error = g_geometry_tracker_mutex.initialize();
    return ;
}

static uint32_t geometry_lock_tracker_ensure_mutex_initialised()
{
    std::call_once(g_geometry_tracker_init_once,
        geometry_lock_tracker_initialize_mutex_once);
    return (g_geometry_tracker_init_error);
}

static uint32_t geometry_lock_tracker_register_wait(pt_thread_id_type thread_identifier,
        const void *owned_object, const void *requested_object,
        ft_bool &out_cycle_detected)
{
    uint32_t lock_error;
    uint32_t initialize_error;

    initialize_error = geometry_lock_tracker_ensure_mutex_initialised();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lock_error = pt_mutex_lock_if_not_null(&g_geometry_tracker_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    ft_size_t index;
    ft_bool found;

    index = 0;
    found = FT_FALSE;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier == thread_identifier)
        {
            g_geometry_wait_records[index].owned_object = owned_object;
            g_geometry_wait_records[index].requested_object = requested_object;
            found = FT_TRUE;
        }
        index += 1;
    }
    if (found == FT_FALSE)
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
            (void)pt_mutex_unlock_if_not_null(&g_geometry_tracker_mutex);
            return (FT_ERR_NO_MEMORY);
        }
    }
    ft_bool cycle_detected;

    cycle_detected = FT_FALSE;
    index = 0;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier != thread_identifier)
        {
            if (g_geometry_wait_records[index].owned_object == requested_object
                && g_geometry_wait_records[index].requested_object == owned_object)
                cycle_detected = FT_TRUE;
        }
        index += 1;
    }
    out_cycle_detected = cycle_detected;
    (void)pt_mutex_unlock_if_not_null(&g_geometry_tracker_mutex);
    return (FT_ERR_SUCCESS);
}

static void geometry_lock_tracker_clear_wait(pt_thread_id_type thread_identifier)
{
    uint32_t lock_error;
    uint32_t initialize_error;

    initialize_error = geometry_lock_tracker_ensure_mutex_initialised();
    if (initialize_error != FT_ERR_SUCCESS)
        return ;
    lock_error = pt_mutex_lock_if_not_null(&g_geometry_tracker_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    ft_size_t index;

    index = 0;
    while (index < g_geometry_wait_records.size())
    {
        if (g_geometry_wait_records[index].thread_identifier == thread_identifier)
        {
            ft_size_t shift_index;

            shift_index = index;
            while (shift_index + 1 < g_geometry_wait_records.size())
            {
                g_geometry_wait_records[shift_index] = g_geometry_wait_records[shift_index + 1];
                shift_index += 1;
            }
            if (!g_geometry_wait_records.empty())
                g_geometry_wait_records.pop_back();
            break ;
        }
        index += 1;
    }
    (void)pt_mutex_unlock_if_not_null(&g_geometry_tracker_mutex);
    return ;
}

static void geometry_lock_tracker_sleep_backoff()
{
    static thread_local ft_bool generator_initialised = FT_FALSE;
    static thread_local std::minstd_rand generator;
    std::uniform_int_distribution<uint32_t> distribution(1, 10);
    uint64_t time_seed;
    uintptr_t address_seed;
    uint32_t combined_seed;
    uint32_t delay_ms;

    if (generator_initialised == FT_FALSE)
    {
        time_seed = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        address_seed = reinterpret_cast<uintptr_t>(&generator);
        combined_seed = static_cast<uint32_t>(time_seed ^ static_cast<uint64_t>(address_seed));
        if (combined_seed == 0)
            combined_seed = static_cast<uint32_t>(address_seed | static_cast<uintptr_t>(1U));
        generator.seed(combined_seed);
        generator_initialised = FT_TRUE;
    }
    delay_ms = distribution(generator);
    pt_thread_sleep(delay_ms);
    return ;
}

uint32_t geometry_lock_tracker_lock_pair(const void *first_object, const void *second_object,
        pt_recursive_mutex &first_mutex, pt_recursive_mutex &second_mutex)
{
    if (first_object == second_object)
    {
        uint32_t self_error;

        self_error = pt_recursive_mutex_lock_if_not_null(&first_mutex);
        if (self_error != FT_ERR_SUCCESS)
            return (self_error);
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
    uint32_t attempt_count;

    attempt_count = 0;
    while (attempt_count < 8192)
    {
        uint32_t lower_error;

        lower_error = pt_recursive_mutex_lock_if_not_null(ordered_first_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        ft_bool cycle_detected;
        uint32_t wait_error;

        cycle_detected = FT_FALSE;
        wait_error = geometry_lock_tracker_register_wait(THREAD_ID,
                ordered_first_object, ordered_second_object, cycle_detected);
        if (wait_error != FT_ERR_SUCCESS)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(ordered_first_mutex);
            return (wait_error);
        }
        uint32_t upper_error;

        upper_error = pt_recursive_mutex_lock_if_not_null(ordered_second_mutex);
        (void)cycle_detected;
        geometry_lock_tracker_clear_wait(THREAD_ID);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(ordered_first_mutex);
            return (upper_error);
        }
        (void)pt_recursive_mutex_unlock_if_not_null(ordered_first_mutex);
        attempt_count = attempt_count + 1;
        geometry_lock_tracker_sleep_backoff();
    }
    return (FT_ERR_MUTEX_ALREADY_LOCKED);
}
