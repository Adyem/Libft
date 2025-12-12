#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Time/time.hpp"

static thread_local bool g_registry_mutex_owned = false;

pthread_mutex_t *pt_lock_tracking::get_registry_mutex(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    static pthread_mutex_t registry_mutex = PTHREAD_MUTEX_INITIALIZER;
#pragma GCC diagnostic pop

    return (&registry_mutex);
}

bool pt_lock_tracking::ensure_registry_mutex_initialized(void)
{
    pt_lock_tracking::get_registry_mutex();
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *pt_lock_tracking::get_thread_infos(void)
{
    static std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *thread_infos_pointer = ft_nullptr;
    void *memory_pointer;

    if (thread_infos_pointer != ft_nullptr)
        return (thread_infos_pointer);
    memory_pointer = std::malloc(sizeof(std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> >));
    if (memory_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    thread_infos_pointer = new(memory_pointer) std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> >();
    ft_errno = FT_ERR_SUCCESSS;
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

pt_mutex_vector pt_lock_tracking::get_owned_mutexes(pt_thread_id_type thread_identifier)
{
    pt_mutex_vector owned_mutexes;
    s_pt_thread_lock_info *info;
    ft_size_t index;
    int lock_error;
    bool lock_acquired;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return (owned_mutexes);
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (owned_mutexes);
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
        info = pt_lock_tracking::find_thread_info(thread_identifier);
    }
    else
        info = pt_lock_tracking::lookup_thread_info(thread_identifier);
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return (owned_mutexes);
            }
            g_registry_mutex_owned = false;
        }
        return (owned_mutexes);
    }
    index = 0;
    while (index < info->owned_mutexes.size())
    {
        owned_mutexes.push_back(info->owned_mutexes[index]);
        index += 1;
    }
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (owned_mutexes);
        }
        g_registry_mutex_owned = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (owned_mutexes);
}

s_pt_thread_lock_info *pt_lock_tracking::find_thread_info(pt_thread_id_type thread_identifier)
{
    ft_size_t index;
    std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *thread_infos;
    s_pt_thread_lock_info *info;

    thread_infos = pt_lock_tracking::get_thread_infos();
    if (thread_infos == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < thread_infos->size())
    {
        info = &(*thread_infos)[index];
        if (info->thread_identifier == thread_identifier)
            return (info);
        index += 1;
    }
    s_pt_thread_lock_info new_info;

    new_info.thread_identifier = thread_identifier;
    new_info.owned_mutexes.clear();
    new_info.waiting_mutex = ft_nullptr;
    new_info.wait_started_ms = 0;
    thread_infos->push_back(new_info);
    return (&thread_infos->back());
}

s_pt_thread_lock_info *pt_lock_tracking::lookup_thread_info(pt_thread_id_type thread_identifier)
{
    ft_size_t index;
    std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *thread_infos;
    s_pt_thread_lock_info *info;

    thread_infos = pt_lock_tracking::get_thread_infos();
    if (thread_infos == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < thread_infos->size())
    {
        info = &(*thread_infos)[index];
        if (info->thread_identifier == thread_identifier)
            return (info);
        index += 1;
    }
    return (ft_nullptr);
}

bool pt_lock_tracking::vector_contains_mutex(const pt_mutex_vector &mutexes, pthread_mutex_t *mutex_pointer)
{
    ft_size_t index;

    index = 0;
    while (index < mutexes.size())
    {
        if (mutexes[index] == mutex_pointer)
            return (true);
        index += 1;
    }
    return (false);
}

bool pt_lock_tracking::vector_contains_thread(const pt_thread_vector &thread_identifiers, pt_thread_id_type thread_identifier)
{
    ft_size_t index;

    index = 0;
    while (index < thread_identifiers.size())
    {
        if (thread_identifiers[index] == thread_identifier)
            return (true);
        index += 1;
    }
    return (false);
}

bool pt_lock_tracking::detect_cycle(const s_pt_thread_lock_info *origin, pthread_mutex_t *requested_mutex, pt_mutex_vector *visited_mutexes, pt_thread_vector *visited_threads)
{
    std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *thread_infos;
    if (pt_lock_tracking::vector_contains_mutex(*visited_mutexes, requested_mutex))
        return (false);
    visited_mutexes->push_back(requested_mutex);
    ft_size_t index;

    thread_infos = pt_lock_tracking::get_thread_infos();
    if (thread_infos == ft_nullptr)
        return (false);
    index = 0;
    while (index < thread_infos->size())
    {
        s_pt_thread_lock_info *info;

        info = &(*thread_infos)[index];
        if (!pt_lock_tracking::vector_contains_mutex(info->owned_mutexes, requested_mutex))
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
        if (pt_lock_tracking::vector_contains_mutex(origin->owned_mutexes, info->waiting_mutex))
            return (true);
        if (!pt_lock_tracking::vector_contains_thread(*visited_threads, info->thread_identifier))
        {
            visited_threads->push_back(info->thread_identifier);
            if (pt_lock_tracking::detect_cycle(origin, info->waiting_mutex, visited_mutexes, visited_threads))
                return (true);
        }
        index += 1;
    }
    return (false);
}

bool pt_lock_tracking::notify_wait(pt_thread_id_type thread_identifier, pthread_mutex_t *requested_mutex, const pt_mutex_vector &owned_mutexes)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    pt_mutex_vector visited_mutexes;
    pt_thread_vector visited_threads;
    bool cycle_detected;
    bool lock_acquired;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return (false);
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return (false);
            }
            g_registry_mutex_owned = false;
        }
        if (!lock_acquired)
            ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    info->owned_mutexes = owned_mutexes;
    if (info->waiting_mutex != requested_mutex)
        info->wait_started_ms = time_now_ms();
    if (info->wait_started_ms == 0)
        info->wait_started_ms = time_now_ms();
    info->waiting_mutex = requested_mutex;
    visited_mutexes.clear();
    visited_threads.clear();
    cycle_detected = pt_lock_tracking::detect_cycle(info, requested_mutex, &visited_mutexes, &visited_threads);
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = false;
    }
    if (cycle_detected)
    {
        ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (false);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

void pt_lock_tracking::notify_acquired(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    bool lock_acquired;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return ;
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return ;
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return ;
            }
            g_registry_mutex_owned = false;
        }
        if (!lock_acquired)
            ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    if (!pt_lock_tracking::vector_contains_mutex(info->owned_mutexes, mutex_pointer))
        info->owned_mutexes.push_back(mutex_pointer);
    info->waiting_mutex = ft_nullptr;
    info->wait_started_ms = 0;
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return ;
        }
        g_registry_mutex_owned = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void pt_lock_tracking::notify_released(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    ft_size_t index;
    bool lock_acquired;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return ;
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return ;
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return ;
            }
            g_registry_mutex_owned = false;
        }
        if (!lock_acquired)
            ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    index = 0;
    while (index < info->owned_mutexes.size())
    {
        if (info->owned_mutexes[index] == mutex_pointer)
        {
            info->owned_mutexes.erase(info->owned_mutexes.begin() + index);
            break ;
        }
        index += 1;
    }
    if (info->waiting_mutex == mutex_pointer)
    {
        info->waiting_mutex = ft_nullptr;
        info->wait_started_ms = 0;
    }
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return ;
        }
        g_registry_mutex_owned = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

bool pt_lock_tracking::snapshot_waiters(pt_lock_wait_snapshot_vector &snapshot)
{
    int lock_error;
    bool lock_acquired;
    std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *thread_infos;
    ft_size_t index;

    snapshot.clear();
    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return (false);
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    thread_infos = pt_lock_tracking::get_thread_infos();
    if (thread_infos == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return (false);
            }
            g_registry_mutex_owned = false;
        }
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    index = 0;
    while (index < thread_infos->size())
    {
        s_pt_thread_lock_info *info;

        info = &(*thread_infos)[index];
        if (info->waiting_mutex != ft_nullptr)
        {
            s_pt_lock_wait_snapshot entry;
            ft_size_t owner_index;
            bool owner_found;

            entry.mutex_pointer = info->waiting_mutex;
            entry.waiting_thread = info->thread_identifier;
            entry.owner_thread = 0;
            entry.wait_started_ms = info->wait_started_ms;
            owner_index = 0;
            owner_found = false;
            while (owner_index < thread_infos->size())
            {
                s_pt_thread_lock_info *owner_info;

                owner_info = &(*thread_infos)[owner_index];
                if (pt_lock_tracking::vector_contains_mutex(owner_info->owned_mutexes, info->waiting_mutex))
                {
                    entry.owner_thread = owner_info->thread_identifier;
                    owner_found = true;
                    owner_index = thread_infos->size();
                }
                else
                    owner_index += 1;
            }
            if (!owner_found)
                entry.owner_thread = 0;
            snapshot.push_back(entry);
        }
        index += 1;
    }
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

#ifdef PT_LOCK_TRACKING_TESTING
bool pt_lock_tracking::get_thread_state(pt_thread_id_type thread_identifier, s_pt_lock_tracking_thread_state &state)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    ft_size_t index;
    bool lock_acquired;

    if (!pt_lock_tracking::ensure_registry_mutex_initialized())
        return (false);
    lock_acquired = false;
    if (!g_registry_mutex_owned)
    {
        lock_error = pthread_mutex_lock(pt_lock_tracking::get_registry_mutex());
        if (lock_error != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = true;
        lock_acquired = true;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    if (info == ft_nullptr)
    {
        if (lock_acquired)
        {
            if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
            {
                ft_errno = FT_ERR_INVALID_STATE;
                return (false);
            }
            g_registry_mutex_owned = false;
        }
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    state.thread_identifier = info->thread_identifier;
    state.waiting_mutex = info->waiting_mutex;
    state.wait_started_ms = info->wait_started_ms;
    state.owned_mutexes.clear();
    index = 0;
    while (index < info->owned_mutexes.size())
    {
        state.owned_mutexes.push_back(info->owned_mutexes[index]);
        index += 1;
    }
    if (lock_acquired)
    {
        if (pthread_mutex_unlock(pt_lock_tracking::get_registry_mutex()) != 0)
        {
            ft_errno = FT_ERR_INVALID_STATE;
            return (false);
        }
        g_registry_mutex_owned = false;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}
#endif
