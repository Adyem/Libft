#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void game_goal_move(game_goal &value)
{
  (void)value.move(value);
  return ;
}

static void game_goal_enable(game_goal &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void game_goal_disable(game_goal &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void game_goal_is_thread_safe(game_goal &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void game_goal_get_target(game_goal &value)
{
  (void)value.get_target();
  return ;
}

static void game_goal_set_target(game_goal &value)
{
  value.set_target(1);
  return ;
}

static void game_goal_get_progress(game_goal &value)
{
  (void)value.get_progress();
  return ;
}

static void game_goal_get_error(game_goal &value)
{
  (void)value.get_error();
  return ;
}

FT_TEST(test_game_goal_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(
      0, expect_sigabrt_on_uninitialised_object<game_goal>(game_goal_move));
  return (1);
}

FT_TEST(test_game_goal_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(
      1, expect_sigabrt_on_uninitialised_object<game_goal>(game_goal_enable));
  return (1);
}

FT_TEST(test_game_goal_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(
      0, expect_sigabrt_on_uninitialised_object<game_goal>(game_goal_disable));
  return (1);
}

FT_TEST(test_game_goal_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_goal>(
                      game_goal_is_thread_safe));
  return (1);
}

FT_TEST(test_game_goal_get_target_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_goal>(
                      game_goal_get_target));
  return (1);
}

FT_TEST(test_game_goal_set_target_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_goal>(
                      game_goal_set_target));
  return (1);
}

FT_TEST(test_game_goal_get_progress_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_goal>(
                      game_goal_get_progress));
  return (1);
}

FT_TEST(test_game_goal_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_goal>(
                      game_goal_get_error));
  return (1);
}
