#include "game_economy_helpers.hpp"
#include "../PThread/pthread.hpp"

void game_economy_sleep_backoff() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    pt_thread_sleep(1);
    ft_errno = entry_errno;
    return ;
}

void game_economy_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int unlock_error;

    unlock_error = ER_SUCCESS;
    if (guard.owns_lock())
    {
        guard.unlock();
        unlock_error = guard.get_error();
    }
    if (unlock_error != ER_SUCCESS)
    {
        ft_errno = unlock_error;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}
