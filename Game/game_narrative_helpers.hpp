#ifndef GAME_NARRATIVE_HELPERS_HPP
# define GAME_NARRATIVE_HELPERS_HPP

#include "../PThread/unique_lock.hpp"
#include "../PThread/mutex.hpp"

void game_narrative_sleep_backoff();
void game_narrative_restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

#endif
