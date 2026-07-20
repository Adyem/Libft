#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../test_internal.hpp"

template <typename TypeName>
static int32_t
expect_sigabrt_on_uninitialised_object(void (*operation)(TypeName &))
{
  return (test_expect_sigabrt_signal_uninitialised<TypeName>(operation));
}

static void catalog_move(game_data_catalog &value)
{
  (void)value.move(value);
  return ;
}

static void catalog_enable(game_data_catalog &value)
{
  (void)value.enable_thread_safety();
  return ;
}

static void catalog_disable(game_data_catalog &value)
{
  (void)value.disable_thread_safety();
  return ;
}

static void catalog_is_thread_safe(game_data_catalog &value)
{
  (void)value.is_thread_safe();
  return ;
}

static void catalog_get_items(game_data_catalog &value)
{
  (void)value.get_item_definitions();
  return ;
}

static void catalog_get_error(game_data_catalog &value)
{
  (void)value.get_error();
  return ;
}

FT_TEST(test_game_data_catalog_move_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_move));
  return (1);
}

FT_TEST(test_game_data_catalog_enable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_enable));
  return (1);
}

FT_TEST(test_game_data_catalog_disable_thread_safety_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_disable));
  return (1);
}

FT_TEST(test_game_data_catalog_is_thread_safe_uninitialised_is_noop)
{
  FT_ASSERT_EQ(0, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_is_thread_safe));
  return (1);
}

FT_TEST(test_game_data_catalog_get_items_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_get_items));
  return (1);
}

FT_TEST(test_game_data_catalog_get_error_uninitialised_aborts)
{
  FT_ASSERT_EQ(1, expect_sigabrt_on_uninitialised_object<game_data_catalog>(
                      catalog_get_error));
  return (1);
}
