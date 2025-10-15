#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

pthread_mutex_t pt_lock_tracking::_registry_mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<s_pt_thread_lock_info> pt_lock_tracking::_thread_infos;

pt_lock_tracking::pt_lock_tracking()
{
    return ;
}

pt_lock_tracking::~pt_lock_tracking()
{
    return ;
}

std::vector<pthread_mutex_t *> pt_lock_tracking::get_owned_mutexes(pt_thread_id_type thread_identifier)
{
    std::vector<pthread_mutex_t *> owned_mutexes;
    int lock_error;
    s_pt_thread_lock_info *info;
    ft_size_t index;

    lock_error = pthread_mutex_lock(&pt_lock_tracking::_registry_mutex);
    if (lock_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (owned_mutexes);
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    index = 0;
    while (index < info->owned_mutexes.size())
    {
        owned_mutexes.push_back(info->owned_mutexes[index]);
        index += 1;
    }
    if (pthread_mutex_unlock(&pt_lock_tracking::_registry_mutex) != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (owned_mutexes);
    }
    ft_errno = ER_SUCCESS;
    return (owned_mutexes);
}

s_pt_thread_lock_info *pt_lock_tracking::find_thread_info(pt_thread_id_type thread_identifier)
{
    ft_size_t index;

    index = 0;
    while (index < pt_lock_tracking::_thread_infos.size())
    {
        if (pt_lock_tracking::_thread_infos[index].thread_identifier == thread_identifier)
            return (&pt_lock_tracking::_thread_infos[index]);
        index += 1;
    }
    s_pt_thread_lock_info new_info;

    new_info.thread_identifier = thread_identifier;
    new_info.owned_mutexes.clear();
    new_info.waiting_mutex = ft_nullptr;
    pt_lock_tracking::_thread_infos.push_back(new_info);
    return (&pt_lock_tracking::_thread_infos.back());
}

bool pt_lock_tracking::vector_contains_mutex(const std::vector<pthread_mutex_t *> &mutexes, pthread_mutex_t *mutex_pointer)
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

bool pt_lock_tracking::vector_contains_thread(const std::vector<pt_thread_id_type> &thread_identifiers, pt_thread_id_type thread_identifier)
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

bool pt_lock_tracking::detect_cycle(const s_pt_thread_lock_info *origin, pthread_mutex_t *requested_mutex, std::vector<pthread_mutex_t *> *visited_mutexes, std::vector<pt_thread_id_type> *visited_threads)
{
    if (pt_lock_tracking::vector_contains_mutex(*visited_mutexes, requested_mutex))
        return (false);
    visited_mutexes->push_back(requested_mutex);
    ft_size_t index;

    index = 0;
    while (index < pt_lock_tracking::_thread_infos.size())
    {
        s_pt_thread_lock_info *info;

        info = &pt_lock_tracking::_thread_infos[index];
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

bool pt_lock_tracking::notify_wait(pt_thread_id_type thread_identifier, pthread_mutex_t *requested_mutex, const std::vector<pthread_mutex_t *> &owned_mutexes)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    std::vector<pthread_mutex_t *> visited_mutexes;
    std::vector<pt_thread_id_type> visited_threads;
    bool cycle_detected;

    lock_error = pthread_mutex_lock(&pt_lock_tracking::_registry_mutex);
    if (lock_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    info->owned_mutexes = owned_mutexes;
    info->waiting_mutex = requested_mutex;
    visited_mutexes.clear();
    visited_threads.clear();
    cycle_detected = pt_lock_tracking::detect_cycle(info, requested_mutex, &visited_mutexes, &visited_threads);
    pthread_mutex_unlock(&pt_lock_tracking::_registry_mutex);
    if (cycle_detected)
    {
        ft_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (false);
    }
    ft_errno = ER_SUCCESS;
    return (true);
}

void pt_lock_tracking::notify_acquired(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;

    lock_error = pthread_mutex_lock(&pt_lock_tracking::_registry_mutex);
    if (lock_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
    if (!pt_lock_tracking::vector_contains_mutex(info->owned_mutexes, mutex_pointer))
        info->owned_mutexes.push_back(mutex_pointer);
    info->waiting_mutex = ft_nullptr;
    pthread_mutex_unlock(&pt_lock_tracking::_registry_mutex);
    ft_errno = ER_SUCCESS;
    return ;
}

void pt_lock_tracking::notify_released(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer)
{
    int lock_error;
    s_pt_thread_lock_info *info;
    ft_size_t index;

    lock_error = pthread_mutex_lock(&pt_lock_tracking::_registry_mutex);
    if (lock_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    info = pt_lock_tracking::find_thread_info(thread_identifier);
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
        info->waiting_mutex = ft_nullptr;
    pthread_mutex_unlock(&pt_lock_tracking::_registry_mutex);
    ft_errno = ER_SUCCESS;
    return ;
}
