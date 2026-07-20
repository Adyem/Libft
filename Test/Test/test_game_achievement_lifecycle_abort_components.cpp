#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void game_achievement_move(game_achievement &value)
{
  (void)value.move(value);
  return ;
}

static void game_achievement_enable(game_achievement &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void game_achievement_disable(game_achievement &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void game_achievement_is_thread_safe(game_achievement &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void game_achievement_get_id(game_achievement &value)
{
  (void)value.get_id();
  return ;
}

static void game_achievement_set_id(game_achievement &value)
{
  value.set_id(1);
  return ;
}

static void game_achievement_get_goal(game_achievement &value)
{
  (void)value.get_goal(1);
  return ;
}

static void game_achievement_get_error(game_achievement &value)
{
  (void)value.get_error();
  return ;
}

static void game_achievement_get_progress(game_achievement &value)
{
  (void)value.get_progress(1);
  return ;
}

static void game_achievement_set_goal(game_achievement &value)
{
  value.set_goal(1, 1);
  return ;
}

static void game_achievement_set_progress(game_achievement &value)
{
  value.set_progress(1, 1);
  return ;
}

static void game_achievement_add_progress(game_achievement &value)
{
  value.add_progress(1, 1);
  return ;
}

FT_TEST(test_game_achievement_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_move));
  return (1);
}

FT_TEST(test_game_achievement_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_enable));
  return (1);
}

FT_TEST(test_game_achievement_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_disable));
  return (1);
}

FT_TEST(test_game_achievement_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_is_thread_safe));
  return (1);
}

FT_TEST(test_game_achievement_get_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_get_id));
  return (1);
}

FT_TEST(test_game_achievement_set_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_set_id));
  return (1);
}

FT_TEST(test_game_achievement_get_goal_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_get_goal));
  return (1);
}

FT_TEST(test_game_achievement_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_get_error));
  return (1);
}

FT_TEST(test_game_achievement_get_progress_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_get_progress));
  return (1);
}

FT_TEST(test_game_achievement_set_goal_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_set_goal));
  return (1);
}

FT_TEST(test_game_achievement_set_progress_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_set_progress));
  return (1);
}

FT_TEST(test_game_achievement_add_progress_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_achievement>(
                      game_achievement_add_progress));
  return (1);
}
