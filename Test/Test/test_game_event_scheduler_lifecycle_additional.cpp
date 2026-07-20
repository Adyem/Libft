#include "../../Modules/Game/game_event_scheduler.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void event_scheduler_initialize_twice(game_event_scheduler &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void event_scheduler_move_uninitialised(game_event_scheduler &value)
{
    game_event_scheduler source;

    (void)value.move(source);
    return ;
}

static void event_scheduler_schedule_event(game_event_scheduler &value)
{
    ft_sharedptr<game_event> event;

    value.schedule_event(event);
    return ;
}

static void event_scheduler_cancel_event(game_event_scheduler &value)
{
    value.cancel_event(1);
    return ;
}

static void event_scheduler_reschedule_event(game_event_scheduler &value)
{
    value.reschedule_event(1, 2);
    return ;
}

static void event_scheduler_update_events(game_event_scheduler &value)
{
    ft_sharedptr<game_world> world;

    value.update_events(world, 1);
    return ;
}

static void event_scheduler_enable_profiling(game_event_scheduler &value)
{
    value.enable_profiling(FT_TRUE);
    return ;
}

static void event_scheduler_profiling_enabled(game_event_scheduler &value)
{
    (void)value.profiling_enabled();
    return ;
}

static void event_scheduler_reset_profile(game_event_scheduler &value)
{
    value.reset_profile();
    return ;
}

static void event_scheduler_snapshot_profile(game_event_scheduler &value)
{
    t_event_scheduler_profile profile;

    value.snapshot_profile(profile);
    return ;
}

static void event_scheduler_dump_events(game_event_scheduler &value)
{
    ft_vector<ft_sharedptr<game_event>> events;

    (void)events.initialize();
    value.dump_events(events);
    return ;
}

static void event_scheduler_size(game_event_scheduler &value)
{
    (void)value.size();
    return ;
}

static void event_scheduler_clear(game_event_scheduler &value)
{
    value.clear();
    return ;
}

static void event_scheduler_enable_thread_safety(game_event_scheduler &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void event_scheduler_disable_thread_safety(game_event_scheduler &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void event_scheduler_is_thread_safe(game_event_scheduler &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void event_scheduler_get_error(game_event_scheduler &value)
{
    (void)value.get_error();
    return ;
}

static void event_scheduler_get_error_str(game_event_scheduler &value)
{
    (void)value.get_error_str();
    return ;
}

static void event_scheduler_destroy(game_event_scheduler &value)
{
    (void)value.destroy();
    return ;
}

FT_TEST(test_event_scheduler_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_initialize_twice));
    return (1);
}

FT_TEST(test_event_scheduler_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_move_uninitialised));
    return (1);
}

FT_TEST(test_event_scheduler_schedule_event_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_schedule_event));
    return (1);
}

FT_TEST(test_event_scheduler_cancel_event_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_cancel_event));
    return (1);
}

FT_TEST(test_event_scheduler_reschedule_event_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_reschedule_event));
    return (1);
}

FT_TEST(test_event_scheduler_update_events_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_update_events));
    return (1);
}

FT_TEST(test_event_scheduler_enable_profiling_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_enable_profiling));
    return (1);
}

FT_TEST(test_event_scheduler_profiling_enabled_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_profiling_enabled));
    return (1);
}

FT_TEST(test_event_scheduler_reset_profile_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_reset_profile));
    return (1);
}

FT_TEST(test_event_scheduler_snapshot_profile_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_snapshot_profile));
    return (1);
}

FT_TEST(test_event_scheduler_dump_events_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_dump_events));
    return (1);
}

FT_TEST(test_event_scheduler_size_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_size));
    return (1);
}

FT_TEST(test_event_scheduler_clear_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_clear));
    return (1);
}

FT_TEST(test_event_scheduler_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_enable_thread_safety));
    return (1);
}

FT_TEST(test_event_scheduler_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_disable_thread_safety));
    return (1);
}

FT_TEST(test_event_scheduler_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_is_thread_safe));
    return (1);
}

FT_TEST(test_event_scheduler_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_get_error));
    return (1);
}

FT_TEST(test_event_scheduler_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_get_error_str));
    return (1);
}

FT_TEST(test_event_scheduler_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_event_scheduler>(
                        event_scheduler_destroy));
    return (1);
}
