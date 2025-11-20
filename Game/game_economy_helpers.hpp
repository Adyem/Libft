#ifndef GAME_ECONOMY_HELPERS_HPP
# define GAME_ECONOMY_HELPERS_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

void game_economy_sleep_backoff() noexcept;
void game_economy_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

#endif
