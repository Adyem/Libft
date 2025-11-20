#include "game_narrative_helpers.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"

void game_narrative_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

void game_narrative_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}
