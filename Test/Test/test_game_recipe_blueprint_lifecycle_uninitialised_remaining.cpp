#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void recipe_enable_thread_safety(game_recipe_blueprint &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void recipe_disable_thread_safety(game_recipe_blueprint &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void recipe_is_thread_safe(game_recipe_blueprint &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void recipe_get_error(game_recipe_blueprint &value)
{
    (void)value.get_error();
    return ;
}

static void recipe_get_error_str(game_recipe_blueprint &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_recipe_blueprint_enable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(recipe_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(recipe_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(recipe_is_thread_safe));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(recipe_get_error));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(recipe_get_error_str));
    return (1);
}
