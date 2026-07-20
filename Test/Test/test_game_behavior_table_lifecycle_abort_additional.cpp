#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/Game/game_behavior_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void table_move(game_behavior_table &value)
{
  (void)value.move(value);
  return ;
}

static void table_enable(game_behavior_table &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void table_disable(game_behavior_table &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void table_is_thread_safe(game_behavior_table &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void table_get_profiles(game_behavior_table &value)
{
  (void)value.get_profiles();
  return ;
}

static void table_set_profiles(game_behavior_table &value)
{
  value.set_profiles(value.get_profiles());
  return ;
}

static void table_get_error(game_behavior_table &value)
{
  (void)value.get_error();
  return ;
}

static void table_get_error_str(game_behavior_table &value)
{
  (void)value.get_error_str();
  return ;
}

static void table_lock(game_behavior_table &value)
{
  ft_bool lock_acquired;

  (void)value.lock(&lock_acquired);
  return ;
}

static void table_unlock(game_behavior_table &value)
{
  value.unlock(FT_FALSE);
  return ;
}

static void table_register_profile(game_behavior_table &value)
{
  game_behavior_profile profile;

  (void)value.register_profile(profile);
  return ;
}

static void table_fetch_profile(game_behavior_table &value)
{
  game_behavior_profile profile;

  (void)value.fetch_profile(1, profile);
  return ;
}

FT_TEST(test_game_behavior_table_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_move));
  return (1);
}

FT_TEST(test_game_behavior_table_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_enable));
  return (1);
}

FT_TEST(test_game_behavior_table_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_disable));
  return (1);
}

FT_TEST(test_game_behavior_table_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_is_thread_safe));
  return (1);
}

FT_TEST(test_game_behavior_table_get_profiles_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_get_profiles));
  return (1);
}

FT_TEST(test_game_behavior_table_set_profiles_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_set_profiles));
  return (1);
}

FT_TEST(test_game_behavior_table_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_get_error));
  return (1);
}

FT_TEST(test_game_behavior_table_get_error_str_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_get_error_str));
  return (1);
}

FT_TEST(test_game_behavior_table_lock_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_lock));
  return (1);
}

FT_TEST(test_game_behavior_table_unlock_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_unlock));
  return (1);
}

FT_TEST(test_game_behavior_table_register_profile_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_register_profile));
  return (1);
}

FT_TEST(test_game_behavior_table_fetch_profile_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_behavior_table>(
                      table_fetch_profile));
  return (1);
}
