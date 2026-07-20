#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void game_buff_move(game_buff &value)
{
  (void)value.move(value);
  return ;
}

static void game_buff_enable(game_buff &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void game_buff_disable(game_buff &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void game_buff_is_thread_safe(game_buff &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void game_buff_get_id(game_buff &value)
{
  (void)value.get_id();
  return ;
}

static void game_buff_set_id(game_buff &value)
{
  value.set_id(1);
  return ;
}

static void game_buff_get_duration(game_buff &value)
{
  (void)value.get_duration();
  return ;
}

static void game_buff_set_duration(game_buff &value)
{
  value.set_duration(1);
  return ;
}

static void game_buff_add_duration(game_buff &value)
{
  value.add_duration(1);
  return ;
}

static void game_buff_sub_duration(game_buff &value)
{
  value.sub_duration(1);
  return ;
}

static void game_buff_get_modifier1(game_buff &value)
{
  (void)value.get_modifier1();
  return ;
}

static void game_buff_set_modifier1(game_buff &value)
{
  value.set_modifier1(1);
  return ;
}

static void game_buff_add_modifier1(game_buff &value)
{
  value.add_modifier1(1);
  return ;
}

static void game_buff_sub_modifier1(game_buff &value)
{
  value.sub_modifier1(1);
  return ;
}

static void game_buff_get_modifier2(game_buff &value)
{
  (void)value.get_modifier2();
  return ;
}

static void game_buff_set_modifier2(game_buff &value)
{
  value.set_modifier2(1);
  return ;
}

static void game_buff_add_modifier2(game_buff &value)
{
  value.add_modifier2(1);
  return ;
}

static void game_buff_sub_modifier2(game_buff &value)
{
  value.sub_modifier2(1);
  return ;
}

static void game_buff_get_modifier3(game_buff &value)
{
  (void)value.get_modifier3();
  return ;
}

static void game_buff_get_error(game_buff &value)
{
  (void)value.get_error();
  return ;
}

FT_TEST(test_game_buff_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(
      0, expect_sigabrt_on_uninitialised_object<game_buff>(game_buff_move));
  return (1);
}

FT_TEST(test_game_buff_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(
      1, expect_sigabrt_on_uninitialised_object<game_buff>(game_buff_enable));
  return (1);
}

FT_TEST(test_game_buff_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(
      0, expect_sigabrt_on_uninitialised_object<game_buff>(game_buff_disable));
  return (1);
}

FT_TEST(test_game_buff_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_is_thread_safe));
  return (1);
}

FT_TEST(test_game_buff_get_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(
      1, expect_sigabrt_on_uninitialised_object<game_buff>(game_buff_get_id));
  return (1);
}

FT_TEST(test_game_buff_set_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(
      1, expect_sigabrt_on_uninitialised_object<game_buff>(game_buff_set_id));
  return (1);
}

FT_TEST(test_game_buff_get_duration_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_get_duration));
  return (1);
}

FT_TEST(test_game_buff_set_duration_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_set_duration));
  return (1);
}

FT_TEST(test_game_buff_add_duration_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_add_duration));
  return (1);
}

FT_TEST(test_game_buff_sub_duration_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_sub_duration));
  return (1);
}

FT_TEST(test_game_buff_get_modifier1_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_get_modifier1));
  return (1);
}

FT_TEST(test_game_buff_set_modifier1_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_set_modifier1));
  return (1);
}

FT_TEST(test_game_buff_add_modifier1_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_add_modifier1));
  return (1);
}

FT_TEST(test_game_buff_sub_modifier1_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_sub_modifier1));
  return (1);
}

FT_TEST(test_game_buff_get_modifier2_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_get_modifier2));
  return (1);
}

FT_TEST(test_game_buff_set_modifier2_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_set_modifier2));
  return (1);
}

FT_TEST(test_game_buff_add_modifier2_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_add_modifier2));
  return (1);
}

FT_TEST(test_game_buff_sub_modifier2_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_sub_modifier2));
  return (1);
}

FT_TEST(test_game_buff_get_modifier3_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_get_modifier3));
  return (1);
}

FT_TEST(test_game_buff_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_buff>(
                      game_buff_get_error));
  return (1);
}
