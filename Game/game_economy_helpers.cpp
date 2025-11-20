#include "game_economy_helpers.hpp"
#include "../PThread/pthread.hpp"

void game_economy_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void game_economy_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}
