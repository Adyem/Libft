#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_experience_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void line_move(game_dialogue_line &value)
{
  (void)value.move(value);
  return ;
}

static void line_get_id(game_dialogue_line &value)
{
  (void)value.get_line_id();
  return ;
}

static void line_set_id(game_dialogue_line &value)
{
  value.set_line_id(1);
  return ;
}

static void line_get_next(game_dialogue_line &value)
{
  (void)value.get_next_line_ids();
  return ;
}

static void line_enable(game_dialogue_line &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void line_disable(game_dialogue_line &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void line_is_thread_safe(game_dialogue_line &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void line_get_error(game_dialogue_line &value)
{
  (void)value.get_error();
  return ;
}

static void line_set_next(game_dialogue_line &value)
{
  value.set_next_line_ids(value.get_next_line_ids());
  return ;
}

static void line_is_initialised(game_dialogue_line &value)
{
  (void)value.is_initialised();
  return ;
}

FT_TEST(test_game_dialogue_line_move_self_uninitialised_is_noop)
{
  FT_ASSERT_EQ(
      0, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(line_move));
  return (1);
}

FT_TEST(test_game_dialogue_line_get_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_get_id));
  return (1);
}

FT_TEST(test_game_dialogue_line_set_id_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_set_id));
  return (1);
}

FT_TEST(test_game_dialogue_line_get_next_ids_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_get_next));
  return (1);
}

FT_TEST(test_game_dialogue_line_enable_thread_safety_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_enable));
  return (1);
}

FT_TEST(test_game_dialogue_line_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_disable));
  return (1);
}

FT_TEST(test_game_dialogue_line_is_thread_safe_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_is_thread_safe));
  return (1);
}

FT_TEST(test_game_dialogue_line_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_get_error));
  return (1);
}

FT_TEST(test_game_dialogue_line_set_next_ids_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_set_next));
  return (1);
}

FT_TEST(test_game_dialogue_line_is_initialised_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_dialogue_line>(
                      line_is_initialised));
  return (1);
}
