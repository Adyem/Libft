#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include "pthread.hpp"
#include <pthread.h>

static thread_local bool g_registry_mutex_owned = false;

int pt_lock_tracking::lock_registry_mutex(void)
{
    try
    {
        pt_lock_tracking::get_registry_mutex()->lock();
    }
    catch (const std::system_error &)
    {
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESSS);
}

int pt_lock_tracking::unlock_registry_mutex(void)
{
    try
    {
        pt_lock_tracking::get_registry_mutex()->unlock();
    }
    catch (const std::system_error &)
    {
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESSS);
}

std::mutex *pt_lock_tracking::get_registry_mutex(void)
{
    static std::mutex registry_mutex;

    return (&registry_mutex);
}

bool pt_lock_tracking::ensure_registry_mutex_initialized(int *error_code)
{
    pt_lock_tracking::get_registry_mutex();
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (true);
}

pt_buffer<s_pt_thread_lock_info> *pt_lock_tracking::get_thread_infos(int *error_code)
{
    static pt_buffer<s_pt_thread_lock_info> *thread_infos_pointer = ft_nullptr;
    void *memory_pointer;

    if (thread_infos_pointer != ft_nullptr)
    {
        if (error_code)
            *error_code = FT_ERR_SUCCESSS;
        return (thread_infos_pointer);
    }
    memory_pointer = std::malloc(sizeof(pt_buffer<s_pt_thread_lock_info>));
    if (memory_pointer == ft_nullptr)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    thread_infos_pointer =
        static_cast<pt_buffer<s_pt_thread_lock_info> *>(memory_pointer);
    new (thread_infos_pointer) pt_buffer<s_pt_thread_lock_info>();
    pt_buffer_init(*thread_infos_pointer);
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (thread_infos_pointer);
}

pt_lock_tracking::pt_lock_tracking()
{
    return ;
}

pt_lock_tracking::~pt_lock_tracking()
{
    return ;
}

pt_mutex_vector pt_lock_tracking::get_owned_mutexes
        (pt_thread_id_type thread_identifier, int *error_code_out)
{
    pt_mutex_vector owned_mutexes;
    s_pt_thread_lock_info *info;
    ft_size_t index;
    int lock_error;
    bool lock_acquired;
    int error_code;

    error_code = FT_ERR_SUCCESSS;
    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
    {
        if (error_code_out)
            *error_code_out = error_code;
        return (owned_mutexes);
    }
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            if (error_code_out)
                *error_code_out = lock_error;
            return (owned_mutexes);
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    if (g_registry_mutex_owned)
        info = pt_lock_tracking::find_thread_info(thread_identifier, &error_code);
    else
        info = pt_lock_tracking::lookup_thread_info(thread_identifier, &error_code);
    if (info == ft_nullptr && error_code != FT_ERR_SUCCESSS)
    {
        if (lock_acquired)
        {
            pt_lock_tracking::unlock_registry_mutex();
            g_registry_mutex_owned = false;
        }
        if (error_code_out)
            *error_code_out = error_code;
        return (owned_mutexes);
    }
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            pt_lock_tracking::unlock_registry_mutex();
            g_registry_mutex_owned = false;
        }
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_STATE;
        return (owned_mutexes);
    }
    pt_buffer_init(owned_mutexes);
    int copy_error = pt_buffer_copy(owned_mutexes, info->owned_mutexes);
    if (copy_error != FT_ERR_SUCCESSS)
    {
        if (lock_acquired)
        {
            pt_lock_tracking::unlock_registry_mutex();
            g_registry_mutex_owned = false;
        }
        if (error_code_out)
            *error_code_out = copy_error;
        pt_buffer_destroy(owned_mutexes);
        return (owned_mutexes);
    }
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    if (error_code_out)
        *error_code_out = FT_ERR_SUCCESSS;
    return (owned_mutexes);
}

s_pt_thread_lock_info *pt_lock_tracking::find_thread_info
    (pt_thread_id_type thread_identifier, int *error_code)
{
    ft_size_t index;
    pt_buffer<s_pt_thread_lock_info> *thread_infos;
    s_pt_thread_lock_info *info;
    s_pt_thread_lock_info new_info;

    thread_infos = pt_lock_tracking::get_thread_infos(error_code);
    if (thread_infos == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < thread_infos->size)
    {
        info = &thread_infos->data[index];
        if (info->thread_identifier == thread_identifier)
            return (info);
        index += 1;
    }
    new_info.thread_identifier = thread_identifier;
    pt_buffer_init(new_info.owned_mutexes);
    new_info.waiting_mutex = ft_nullptr;
    new_info.wait_started_ms = 0;
    int push_error = pt_buffer_push(*thread_infos, new_info);
    if (push_error != FT_ERR_SUCCESSS)
    {
        if (error_code)
            *error_code = push_error;
        return (ft_nullptr);
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (&thread_infos->data[thread_infos->size - 1]);
}

s_pt_thread_lock_info *pt_lock_tracking::lookup_thread_info(pt_thread_id_type
        thread_identifier, int *error_code)
{
    ft_size_t index;
    pt_buffer<s_pt_thread_lock_info> *thread_infos;
    s_pt_thread_lock_info *info;

    thread_infos = pt_lock_tracking::get_thread_infos(error_code);
    if (thread_infos == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < thread_infos->size)
    {
        info = &thread_infos->data[index];
        if (info->thread_identifier == thread_identifier)
            return (info);
        index += 1;
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (ft_nullptr);
}

bool pt_lock_tracking::vector_contains_mutex(const pt_mutex_vector &mutexes,
        const void *mutex_pointer)
{
    ft_size_t index;

    index = 0;
    while (index < mutexes.size)
    {
        if (mutexes.data[index] == mutex_pointer)
            return (true);
        index += 1;
    }
    return (false);
}

bool pt_lock_tracking::vector_contains_thread(const pt_thread_vector
        &thread_identifiers, pt_thread_id_type thread_identifier)
{
    ft_size_t index;

    index = 0;
    while (index < thread_identifiers.size)
    {
        if (thread_identifiers.data[index] == thread_identifier)
            return (true);
        index += 1;
    }
    return (false);
}

bool pt_lock_tracking::detect_cycle(const s_pt_thread_lock_info *origin,
        const void *requested_mutex, pt_mutex_vector *visited_mutexes,
        pt_thread_vector *visited_threads)
{
    pt_buffer<s_pt_thread_lock_info> *thread_infos;
    int error_code;

    if (pt_lock_tracking::vector_contains_mutex(*visited_mutexes, requested_mutex))
        return (false);
    if (pt_buffer_push(*visited_mutexes, requested_mutex) != FT_ERR_SUCCESSS)
        return (false);
    error_code = FT_ERR_SUCCESSS;
    thread_infos = pt_lock_tracking::get_thread_infos(&error_code);
    if (thread_infos == ft_nullptr)
        return (false);
    ft_size_t index = 0;
    while (index < thread_infos->size)
    {
        s_pt_thread_lock_info *info = &thread_infos->data[index];

        if (!pt_lock_tracking::vector_contains_mutex(info->owned_mutexes,
                    requested_mutex))
        {
            index += 1;
            continue ;
        }
        if (info->thread_identifier == origin->thread_identifier)
        {
            index += 1;
            continue ;
        }
        if (info->waiting_mutex == ft_nullptr)
        {
            index += 1;
            continue ;
        }
        if (pt_lock_tracking::vector_contains_mutex(origin->owned_mutexes,
                    info->waiting_mutex))
            return (true);
        if (!pt_lock_tracking::vector_contains_thread(*visited_threads,
                    info->thread_identifier))
        {
            if (pt_buffer_push(*visited_threads, info->thread_identifier)
                    != FT_ERR_SUCCESSS)
                return (false);
            if (pt_lock_tracking::detect_cycle(origin, info->waiting_mutex,
                        visited_mutexes, visited_threads))
                return (true);
        }
        index += 1;
    }
    return (false);
}

int pt_lock_tracking::notify_wait(pt_thread_id_type thread_identifier,
        const void *requested_mutex, const pt_mutex_vector &owned_mutexes)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    bool lock_acquired = false;
    int error_code = FT_ERR_SUCCESSS;
    int result_code = FT_ERR_SUCCESSS;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
        return (error_code);
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier, &error_code);
    if (info == ft_nullptr)
    {
        if (!lock_acquired && error_code == FT_ERR_SUCCESSS)
            result_code = FT_ERR_INVALID_STATE;
        else
            result_code = error_code;
    }
    else
    {
        pt_buffer_destroy(info->owned_mutexes);
        pt_buffer_init(info->owned_mutexes);
        int copy_error = pt_buffer_copy(info->owned_mutexes, owned_mutexes);
        if (copy_error != FT_ERR_SUCCESSS)
        {
            result_code = copy_error;
        }
        else
        {
            if (info->waiting_mutex != requested_mutex)
                info->wait_started_ms = time_now_ms();
            if (info->wait_started_ms == 0)
                info->wait_started_ms = time_now_ms();
            info->waiting_mutex = requested_mutex;
            pt_mutex_vector visited_mutexes_inner;
            pt_thread_vector visited_threads_inner;
            pt_buffer_init(visited_mutexes_inner);
            pt_buffer_init(visited_threads_inner);
            bool cycle_detected = pt_lock_tracking::detect_cycle(info, requested_mutex,
                    &visited_mutexes_inner, &visited_threads_inner);
            if (cycle_detected)
                result_code = FT_ERR_MUTEX_ALREADY_LOCKED;
            else
                result_code = FT_ERR_SUCCESSS;
            pt_buffer_destroy(visited_threads_inner);
            pt_buffer_destroy(visited_mutexes_inner);
        }
    }
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    return (result_code);
}

int pt_lock_tracking::notify_acquired(pt_thread_id_type thread_identifier,
        const void *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    bool lock_acquired = false;
    int error_code = FT_ERR_SUCCESSS;
    int result_code = FT_ERR_SUCCESSS;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
        return (error_code);
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier, &error_code);
    if (info == ft_nullptr)
    {
        if (!lock_acquired && error_code == FT_ERR_SUCCESSS)
            result_code = FT_ERR_INVALID_STATE;
        else
            result_code = error_code;
        goto cleanup_acquired;
    }
    if (!pt_lock_tracking::vector_contains_mutex(info->owned_mutexes, mutex_pointer))
    {
        int push_error = pt_buffer_push(info->owned_mutexes, mutex_pointer);
        if (push_error != FT_ERR_SUCCESSS)
        {
            result_code = push_error;
            goto cleanup_acquired;
        }
    }
    info->waiting_mutex = ft_nullptr;
    info->wait_started_ms = 0;
cleanup_acquired:
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    return (result_code);
}

int pt_lock_tracking::notify_released(pt_thread_id_type thread_identifier,
        const void *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    ft_size_t index;
    bool lock_acquired = false;
    int error_code = FT_ERR_SUCCESSS;
    int result_code = FT_ERR_SUCCESSS;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
        return (error_code);
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier, &error_code);
    if (info == ft_nullptr)
    {
        if (!lock_acquired && error_code == FT_ERR_SUCCESSS)
            result_code = FT_ERR_INVALID_STATE;
        else
            result_code = error_code;
        goto cleanup_released;
    }
    index = 0;
    while (index < info->owned_mutexes.size)
    {
        if (info->owned_mutexes.data[index] == mutex_pointer)
        {
            pt_buffer_erase(info->owned_mutexes, index);
            break ;
        }
        index += 1;
    }
    if (info->waiting_mutex == mutex_pointer)
    {
        info->waiting_mutex = ft_nullptr;
        info->wait_started_ms = 0;
    }
cleanup_released:
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    return (result_code);
}

int pt_lock_tracking::snapshot_waiters(pt_lock_wait_snapshot_vector &snapshot)
{
    int lock_error;
    bool lock_acquired = false;
    pt_buffer<s_pt_thread_lock_info> *thread_infos;
    ft_size_t index;
    int error_code = FT_ERR_SUCCESSS;
    int result_code = FT_ERR_SUCCESSS;

    pt_buffer_clear(snapshot);
    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
        return (error_code);
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    thread_infos = pt_lock_tracking::get_thread_infos(&error_code);
    if (thread_infos == ft_nullptr)
    {
        if (error_code != FT_ERR_SUCCESSS)
            result_code = error_code;
        else
            result_code = FT_ERR_INVALID_STATE;
        goto cleanup_snapshot;
    }
    index = 0;
    while (index < thread_infos->size)
    {
        s_pt_thread_lock_info *info = &thread_infos->data[index];

        if (info->waiting_mutex != ft_nullptr)
        {
            s_pt_lock_wait_snapshot entry;
            ft_size_t owner_index = 0;
            bool owner_found = false;

            entry.mutex_pointer = info->waiting_mutex;
            entry.waiting_thread = info->thread_identifier;
            entry.owner_thread = 0;
            entry.wait_started_ms = info->wait_started_ms;
            while (owner_index < thread_infos->size)
            {
                s_pt_thread_lock_info *owner_info = &thread_infos->data[owner_index];

                if (pt_lock_tracking::vector_contains_mutex(owner_info->owned_mutexes,
                            info->waiting_mutex))
                {
                    entry.owner_thread = owner_info->thread_identifier;
                    owner_found = true;
                    break ;
                }
                owner_index += 1;
            }
            if (!owner_found)
                entry.owner_thread = 0;
            int push_error = pt_buffer_push(snapshot, entry);
            if (push_error != FT_ERR_SUCCESSS)
            {
                result_code = push_error;
                goto cleanup_snapshot;
            }
        }
        index += 1;
    }
    result_code = FT_ERR_SUCCESSS;
cleanup_snapshot:
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    return (result_code);
}

int pt_lock_tracking::get_thread_state(pt_thread_id_type thread_identifier,
        s_pt_lock_tracking_thread_state &state)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    bool lock_acquired = false;
    int error_code = FT_ERR_SUCCESSS;
    int result_code = FT_ERR_SUCCESSS;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized(&error_code))
        return (error_code);
    if (!g_registry_mutex_owned)
    {
        lock_error = pt_lock_tracking::lock_registry_mutex();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier, &error_code);
    if (info == ft_nullptr)
    {
        if (!lock_acquired && error_code == FT_ERR_SUCCESSS)
            result_code = FT_ERR_INVALID_STATE;
        else
            result_code = error_code;
        goto cleanup_state;
    }
    state.thread_identifier = info->thread_identifier;
    state.waiting_mutex = info->waiting_mutex;
    state.wait_started_ms = info->wait_started_ms;
    int copy_error = pt_buffer_copy(state.owned_mutexes, info->owned_mutexes);
    if (copy_error != FT_ERR_SUCCESSS)
        result_code = copy_error;
cleanup_state:
    if (lock_acquired)
    {
        pt_lock_tracking::unlock_registry_mutex();
        g_registry_mutex_owned = false;
    }
    return (result_code);
}
