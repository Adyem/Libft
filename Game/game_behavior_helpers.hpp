#ifndef GAME_BEHAVIOR_HELPERS_HPP
# define GAME_BEHAVIOR_HELPERS_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

void game_behavior_sleep_backoff() noexcept;
void game_behavior_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

#endif
