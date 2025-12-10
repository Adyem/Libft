#include "game_narrative_helpers.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"

void game_narrative_sleep_backoff()
{
    pt_thread_sleep(1);
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

void game_narrative_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int unlock_error;

    unlock_error = FT_ER_SUCCESSS;
    if (guard.owns_lock())
    {
        guard.unlock();
        unlock_error = guard.get_error();
    }
    if (unlock_error != FT_ER_SUCCESSS)
    {
        ft_errno = unlock_error;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}
