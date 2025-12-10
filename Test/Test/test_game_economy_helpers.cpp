#include "../../Game/game_economy_helpers.hpp"
#include "../../PThread/mutex.hpp"
#include "../../PThread/unique_lock.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_economy_restore_errno_unlocks_guard_and_sets_success, "restore errno unlocks guard and resets errno to success")
{
    pt_mutex mutex;
    ft_unique_lock<pt_mutex> guard(mutex);
    int entry_errno;

    entry_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = FT_ERR_CONFIGURATION;
    game_economy_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, guard.get_error());
    return (1);
}

FT_TEST(test_game_economy_restore_errno_without_lock_sets_success, "restore errno resets errno when guard does not own lock")
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;

    entry_errno = FT_ERR_ALREADY_EXISTS;
    ft_errno = FT_ERR_IO;
    game_economy_restore_errno(guard, entry_errno);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(false, guard.owns_lock());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, guard.get_error());
    return (1);
}

FT_TEST(test_game_economy_sleep_backoff_resets_errno, "sleep backoff clears errno to success")
{
    ft_errno = FT_ERR_NOT_FOUND;
    game_economy_sleep_backoff();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
