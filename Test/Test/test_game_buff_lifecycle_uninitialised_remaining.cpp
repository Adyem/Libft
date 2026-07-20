#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void buff_enable_thread_safety(game_buff &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void buff_disable_thread_safety(game_buff &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void buff_is_thread_safe(game_buff &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void buff_lock(game_buff &value)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}

static void buff_unlock(game_buff &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void buff_get_error(game_buff &value)
{
    (void)value.get_error();
    return ;
}

static void buff_get_error_str(game_buff &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_buff_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(buff_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_buff_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_buff>(buff_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_buff_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_buff>(buff_is_thread_safe));
    return (1);
}

FT_TEST(test_game_buff_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(buff_lock));
    return (1);
}

FT_TEST(test_game_buff_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(buff_unlock));
    return (1);
}

FT_TEST(test_game_buff_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(buff_get_error));
    return (1);
}

FT_TEST(test_game_buff_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(buff_get_error_str));
    return (1);
}

