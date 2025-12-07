#include "../../Game/game_behavior_helpers.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/unique_lock.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_behavior_restore_errno_unlocks_guard_and_resets_errno,
    "restore errno unlocks guard and resets errno to success")
{
    pt_mutex mutex;
    ft_unique_lock<pt_mutex> guard(mutex);
    int entry_errno;

    entry_errno = ER_SUCCESS;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    game_behavior_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
    return (1);
}

FT_TEST(test_game_behavior_restore_errno_without_lock_resets_errno,
    "restore errno resets errno to success when guard does not own lock")
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;

    entry_errno = ER_SUCCESS;
    ft_errno = FT_ERR_ALREADY_EXISTS;
    game_behavior_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
    return (1);
}

FT_TEST(test_game_behavior_sleep_backoff_resets_errno, "sleep backoff resets errno to success")
{
    ft_errno = FT_ERR_NOT_FOUND;
    game_behavior_sleep_backoff();
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
