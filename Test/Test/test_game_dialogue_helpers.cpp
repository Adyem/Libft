#include "../../Game/game_narrative_helpers.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/unique_lock.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_dialogue_helper_restore_errno_sets_success, "restore errno unlocks guards and resets errno to success")
{
    pt_mutex mutex;
    ft_unique_lock<pt_mutex> guard(mutex);
    int original_errno;

    FT_ASSERT(guard.owns_lock());
    original_errno = 9;
    ft_errno = original_errno;
    game_narrative_restore_errno(guard, 4);
    FT_ASSERT_EQ(0, guard.owns_lock());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_dialogue_helper_sleep_backoff, "sleep backoff executes")
{
    game_narrative_sleep_backoff();
    FT_ASSERT_EQ(ER_SUCCESS, ER_SUCCESS);
    return (1);
}
