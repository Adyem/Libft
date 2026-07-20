#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_debuff_additional_add_modifier2(game_debuff &value)
{
    value.add_modifier2(1);
    return ;
}

static void game_debuff_additional_add_modifier1(game_debuff &value)
{
    value.add_modifier1(1);
    return ;
}

static void game_debuff_additional_sub_modifier1(game_debuff &value)
{
    value.sub_modifier1(1);
    return ;
}

static void game_debuff_additional_sub_modifier2(game_debuff &value)
{
    value.sub_modifier2(1);
    return ;
}

static void game_debuff_additional_get_modifier3(game_debuff &value)
{
    (void)value.get_modifier3();
    return ;
}

static void game_debuff_additional_set_modifier3(game_debuff &value)
{
    value.set_modifier3(1);
    return ;
}

static void game_debuff_additional_add_modifier3(game_debuff &value)
{
    value.add_modifier3(1);
    return ;
}

static void game_debuff_additional_sub_modifier3(game_debuff &value)
{
    value.sub_modifier3(1);
    return ;
}

static void game_debuff_additional_get_modifier4(game_debuff &value)
{
    (void)value.get_modifier4();
    return ;
}

static void game_debuff_additional_set_modifier4(game_debuff &value)
{
    value.set_modifier4(1);
    return ;
}

static void game_debuff_additional_add_modifier4(game_debuff &value)
{
    value.add_modifier4(1);
    return ;
}

static void game_debuff_additional_sub_modifier4(game_debuff &value)
{
    value.sub_modifier4(1);
    return ;
}

static void game_debuff_additional_enable_thread_safety(game_debuff &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_debuff_additional_lock(game_debuff &value)
{
    ft_bool lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}

static void game_debuff_additional_unlock(game_debuff &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void game_debuff_additional_get_error(game_debuff &value)
{
    (void)value.get_error();
    return ;
}

static void game_debuff_additional_get_error_str(game_debuff &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_debuff_additional_initialize_twice(game_debuff &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_debuff_add_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_add_modifier2));
    return (1);
}

FT_TEST(test_game_debuff_add_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_add_modifier1));
    return (1);
}

FT_TEST(test_game_debuff_sub_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_sub_modifier1));
    return (1);
}

FT_TEST(test_game_debuff_sub_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_sub_modifier2));
    return (1);
}

FT_TEST(test_game_debuff_get_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_get_modifier3));
    return (1);
}

FT_TEST(test_game_debuff_set_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_set_modifier3));
    return (1);
}

FT_TEST(test_game_debuff_add_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_add_modifier3));
    return (1);
}

FT_TEST(test_game_debuff_sub_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_sub_modifier3));
    return (1);
}

FT_TEST(test_game_debuff_get_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_get_modifier4));
    return (1);
}

FT_TEST(test_game_debuff_set_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_set_modifier4));
    return (1);
}

FT_TEST(test_game_debuff_add_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_add_modifier4));
    return (1);
}

FT_TEST(test_game_debuff_sub_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_sub_modifier4));
    return (1);
}

FT_TEST(test_game_debuff_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_debuff_is_thread_safe_uninitialised_is_safe)
{
    game_debuff value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_debuff_disable_thread_safety_uninitialised_is_safe)
{
    game_debuff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_debuff_lock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_lock));
    return (1);
}

FT_TEST(test_game_debuff_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_unlock));
    return (1);
}

FT_TEST(test_game_debuff_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_get_error));
    return (1);
}

FT_TEST(test_game_debuff_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_get_error_str));
    return (1);
}

FT_TEST(test_game_debuff_destroy_uninitialised_is_safe)
{
    game_debuff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_debuff_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_debuff>(
                        game_debuff_additional_initialize_twice));
    return (1);
}
