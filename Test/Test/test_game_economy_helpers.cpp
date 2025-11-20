#include "../../Game/game_economy_helpers.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/unique_lock.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_economy_restore_errno_unlocks_guard, "restore errno unlocks guard and propagates code")
{
    pt_mutex mutex;
    ft_unique_lock<pt_mutex> guard(mutex);
    int entry_errno;

    entry_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = ER_SUCCESS;
    game_economy_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(entry_errno, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
    return (1);
}

FT_TEST(test_game_economy_restore_errno_without_lock, "restore errno when guard does not own lock")
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;

    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = ER_SUCCESS;
    game_economy_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(entry_errno, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
    return (1);
}

FT_TEST(test_game_economy_sleep_backoff_preserves_errno, "sleep backoff does not modify errno")
{
    ft_errno = FT_ERR_NOT_FOUND;
    game_economy_sleep_backoff();
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}
