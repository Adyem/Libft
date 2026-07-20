#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void region_initialize_twice(game_region_definition &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void region_copy_initialize_uninitialised(game_region_definition &value)
{
    game_region_definition source;

    (void)value.initialize(source);
    return ;
}

static void region_move_initialize_uninitialised(game_region_definition &value)
{
    game_region_definition source;

    (void)value.initialize(static_cast<game_region_definition &&>(source));
    return ;
}

static void region_move_uninitialised(game_region_definition &value)
{
    game_region_definition source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_region_definition_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_region_definition>(
                        region_initialize_twice));
    return (1);
}

FT_TEST(test_game_region_definition_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_region_definition>(
                        region_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_region_definition_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_region_definition>(
                        region_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_region_definition_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_region_definition>(
                        region_move_uninitialised));
    return (1);
}

FT_TEST(test_game_region_definition_initialize_default_values)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_region_id());
    FT_ASSERT_EQ(0, value.get_recommended_level());
    return (1);
}

FT_TEST(test_game_region_definition_initialize_with_values)
{
    game_region_definition value;
    ft_string name;
    ft_string description;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, name.initialize("Forest"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("Green region"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(3, name, description, 7));
    FT_ASSERT_EQ(3, value.get_region_id());
    FT_ASSERT(value.get_name() == name);
    FT_ASSERT(value.get_description() == description);
    FT_ASSERT_EQ(7, value.get_recommended_level());
    return (1);
}

FT_TEST(test_game_region_definition_set_region_id)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_region_id(8);
    FT_ASSERT_EQ(8, value.get_region_id());
    return (1);
}

FT_TEST(test_game_region_definition_set_name)
{
    game_region_definition value;
    ft_string name;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, name.initialize("Desert"));
    value.set_name(name);
    FT_ASSERT(value.get_name() == name);
    return (1);
}

FT_TEST(test_game_region_definition_set_description)
{
    game_region_definition value;
    ft_string description;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("Dry region"));
    value.set_description(description);
    FT_ASSERT(value.get_description() == description);
    return (1);
}

FT_TEST(test_game_region_definition_set_recommended_level)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_recommended_level(12);
    FT_ASSERT_EQ(12, value.get_recommended_level());
    return (1);
}

FT_TEST(test_game_region_definition_thread_safety_enable_disable_cycle)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_region_definition_lock_unlock_initialised)
{
    game_region_definition value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_region_definition_error_after_success)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_region_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_region_definition_destroy_twice_is_safe)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_region_definition_reinitialize_after_destroy)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_region_id(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_region_id());
    return (1);
}

FT_TEST(test_game_region_definition_self_move_is_safe)
{
    game_region_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_region_id(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(10, value.get_region_id());
    return (1);
}

FT_TEST(test_game_region_definition_empty_strings_are_preserved)
{
    game_region_definition value;
    ft_string empty;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, empty.initialize());
    value.set_name(empty);
    value.set_description(empty);
    FT_ASSERT(value.get_name() == empty);
    FT_ASSERT(value.get_description() == empty);
    return (1);
}

FT_TEST(test_game_region_definition_copy_keeps_metadata)
{
    game_region_definition source;
    game_region_definition destination;
    ft_string name;
    ft_string description;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, name.initialize("Mountain"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("High region"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(5, name, description, 11));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(5, destination.get_region_id());
    FT_ASSERT_EQ(11, destination.get_recommended_level());
    return (1);
}
