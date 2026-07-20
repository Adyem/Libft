#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void profile_move(game_behavior_profile &value)
{
  (void)value.move(value);
  return ;
}

static void profile_enable(game_behavior_profile &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void profile_disable(game_behavior_profile &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void profile_is_thread_safe(game_behavior_profile &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void profile_get_id(game_behavior_profile &value)
{
  (void)value.get_profile_id();
  return ;
}

static void profile_set_id(game_behavior_profile &value)
{
  value.set_profile_id(1);
  return ;
}

static void profile_get_aggression(game_behavior_profile &value)
{
  (void)value.get_aggression_weight();
  return ;
}

static void profile_set_aggression(game_behavior_profile &value)
{
  value.set_aggression_weight(1.0);
  return ;
}

static void profile_get_caution(game_behavior_profile &value)
{
  (void)value.get_caution_weight();
  return ;
}

static void profile_set_caution(game_behavior_profile &value)
{
  value.set_caution_weight(1.0);
  return ;
}

static void profile_get_actions(game_behavior_profile &value)
{
  (void)value.get_actions();
  return ;
}

static void profile_get_error(game_behavior_profile &value)
{
  (void)value.get_error();
  return ;
}

FT_TEST(test_game_behavior_profile_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_move));
  return (1);
}

FT_TEST(test_game_behavior_profile_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_enable));
  return (1);
}

FT_TEST(test_game_behavior_profile_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_disable));
  return (1);
}

FT_TEST(test_game_behavior_profile_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_is_thread_safe));
  return (1);
}

FT_TEST(test_game_behavior_profile_get_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_get_id));
  return (1);
}

FT_TEST(test_game_behavior_profile_set_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_set_id));
  return (1);
}

FT_TEST(test_game_behavior_profile_get_aggression_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_get_aggression));
  return (1);
}

FT_TEST(test_game_behavior_profile_set_aggression_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_set_aggression));
  return (1);
}

FT_TEST(test_game_behavior_profile_get_caution_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_get_caution));
  return (1);
}

FT_TEST(test_game_behavior_profile_set_caution_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_set_caution));
  return (1);
}

FT_TEST(test_game_behavior_profile_get_actions_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_get_actions));
  return (1);
}

FT_TEST(test_game_behavior_profile_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_profile>(
                      profile_get_error));
  return (1);
}
