#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_event_get_id(game_event &value)
{
    (void)value.get_id();
    return ;
}

static void game_event_set_id(game_event &value)
{
    value.set_id(1);
    return ;
}

static void game_event_get_duration(game_event &value)
{
    (void)value.get_duration();
    return ;
}

static void game_event_set_duration(game_event &value)
{
    value.set_duration(1);
    return ;
}

static void game_event_add_duration(game_event &value)
{
    (void)value.add_duration(1);
    return ;
}

static void game_event_sub_duration(game_event &value)
{
    value.sub_duration(1);
    return ;
}

static void game_event_enable_thread_safety(game_event &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_event_disable_thread_safety(game_event &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_event_is_thread_safe(game_event &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_event_get_error(game_event &value)
{
    (void)value.get_error();
    return ;
}

static void game_event_get_error_str(game_event &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_event_move_uninitialised(game_event &value)
{
    game_event source;

    (void)value.move(source);
    return ;
}

static void game_event_initialize_twice(game_event &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_event_destroy(game_event &value)
{
    (void)value.destroy();
    return ;
}

static void game_event_get_modifier1(game_event &value)
{
    (void)value.get_modifier1();
    return ;
}

static void game_event_set_modifier1(game_event &value)
{
    value.set_modifier1(1);
    return ;
}

static void game_event_get_modifier2(game_event &value)
{
    (void)value.get_modifier2();
    return ;
}

static void game_event_set_modifier2(game_event &value)
{
    value.set_modifier2(1);
    return ;
}

static void game_event_get_modifier3(game_event &value)
{
    (void)value.get_modifier3();
    return ;
}

static void game_event_set_modifier3(game_event &value)
{
    value.set_modifier3(1);
    return ;
}

static void game_event_get_modifier4(game_event &value)
{
    (void)value.get_modifier4();
    return ;
}

static void game_event_set_modifier4(game_event &value)
{
    value.set_modifier4(1);
    return ;
}

static void game_event_get_callback(game_event &value)
{
    (void)value.get_callback();
    return ;
}

static void game_event_set_callback(game_event &value)
{
    ft_function<void(game_world &, game_event &)> callback;

    value.set_callback(
        static_cast<ft_function<void(game_world &, game_event &)> &&>(
            callback));
    return ;
}

FT_TEST(test_game_event_get_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_event>(game_event_get_id));
    return (1);
}

FT_TEST(test_game_event_set_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_event>(game_event_set_id));
    return (1);
}

FT_TEST(test_game_event_get_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_get_duration));
    return (1);
}

FT_TEST(test_game_event_set_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_set_duration));
    return (1);
}

FT_TEST(test_game_event_add_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_add_duration));
    return (1);
}

FT_TEST(test_game_event_sub_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_sub_duration));
    return (1);
}

FT_TEST(test_game_event_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event>(
                        game_event_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_event_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event>(
                        game_event_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_event_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event>(
                        game_event_is_thread_safe));
    return (1);
}

FT_TEST(test_game_event_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_get_error));
    return (1);
}

FT_TEST(test_game_event_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_event>(game_event_get_error_str));
    return (1);
}

FT_TEST(test_game_event_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event>(
                        game_event_move_uninitialised));
    return (1);
}

FT_TEST(test_game_event_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event>(
                        game_event_initialize_twice));
    return (1);
}

FT_TEST(test_game_event_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0,
                 expect_game_lifecycle_sigabrt<game_event>(game_event_destroy));
    return (1);
}

FT_TEST(test_game_event_get_modifier1_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_get_modifier1));
    return (1);
}

FT_TEST(test_game_event_set_modifier1_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_set_modifier1));
    return (1);
}

FT_TEST(test_game_event_get_modifier2_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_get_modifier2));
    return (1);
}

FT_TEST(test_game_event_set_modifier2_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_set_modifier2));
    return (1);
}

FT_TEST(test_game_event_get_modifier3_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_get_modifier3));
    return (1);
}

FT_TEST(test_game_event_set_modifier3_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_set_modifier3));
    return (1);
}

FT_TEST(test_game_event_get_modifier4_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_get_modifier4));
    return (1);
}

FT_TEST(test_game_event_set_modifier4_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_set_modifier4));
    return (1);
}

FT_TEST(test_game_event_get_callback_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_get_callback));
    return (1);
}

FT_TEST(test_game_event_set_callback_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_event>(game_event_set_callback));
    return (1);
}
