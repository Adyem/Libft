#include "../../Modules/Game/game_behavior_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_behavior_table_additional_initialize_twice(game_behavior_table &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_behavior_table_additional_copy_initialize_uninitialised(
    game_behavior_table &value)
{
    game_behavior_table source;

    (void)value.initialize(source);
    return ;
}

static void game_behavior_table_additional_move_initialize_uninitialised(
    game_behavior_table &value)
{
    game_behavior_table source;

    (void)value.initialize(static_cast<game_behavior_table &&>(source));
    return ;
}

static void
game_behavior_table_additional_move_uninitialised(game_behavior_table &value)
{
    game_behavior_table source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_behavior_table_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_table>(
                        game_behavior_table_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_behavior_table_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_behavior_table>(
               game_behavior_table_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_table_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_behavior_table>(
               game_behavior_table_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_table_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_behavior_table>(
                        game_behavior_table_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_behavior_table_get_profiles_mutable_initialised)
{
    game_behavior_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_get_profiles_const_initialised)
{
    game_behavior_table value;
    const game_behavior_table &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_set_empty_profiles_initialised)
{
    game_behavior_table value;
    ft_map<int32_t, game_behavior_profile> profiles;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profiles.initialize());
    value.set_profiles(profiles);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_EQ(0, value.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_register_profile_initialised)
{
    game_behavior_table table;
    game_behavior_profile profile;
    ft_vector<game_behavior_action> actions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(3, 0.6, 0.4, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(1, table.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_fetch_profile_initialised)
{
    game_behavior_table table;
    game_behavior_profile source;
    game_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_profile_id(6);
    source.set_aggression_weight(0.9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(6, fetched));
    FT_ASSERT_EQ(6, fetched.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(0.9, fetched.get_aggression_weight());
    return (1);
}

FT_TEST(test_game_behavior_table_fetch_missing_profile_reports_error)
{
    game_behavior_table table;
    game_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.fetch_profile(99, fetched));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, table.get_error());
    return (1);
}

FT_TEST(test_game_behavior_table_register_profile_replaces_entry)
{
    game_behavior_table table;
    game_behavior_profile first_profile;
    game_behavior_profile second_profile;
    game_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_profile.initialize());
    first_profile.set_profile_id(5);
    first_profile.set_aggression_weight(0.2);
    second_profile.set_profile_id(5);
    second_profile.set_aggression_weight(0.8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(first_profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(second_profile));
    FT_ASSERT_EQ(1, table.get_profiles().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(5, fetched));
    FT_ASSERT_DOUBLE_EQ(0.8, fetched.get_aggression_weight());
    return (1);
}

FT_TEST(test_game_behavior_table_register_multiple_profiles)
{
    game_behavior_table table;
    game_behavior_profile first_profile;
    game_behavior_profile second_profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first_profile.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_profile.initialize());
    first_profile.set_profile_id(1);
    second_profile.set_profile_id(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(first_profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(second_profile));
    FT_ASSERT_EQ(2, table.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_register_profile_preserves_values)
{
    game_behavior_table table;
    game_behavior_profile profile;
    game_behavior_profile fetched;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    profile.set_profile_id(7);
    profile.set_caution_weight(0.35);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(7, fetched));
    FT_ASSERT_DOUBLE_EQ(0.35, fetched.get_caution_weight());
    return (1);
}

FT_TEST(test_game_behavior_table_thread_safety_enable_disable_cycle)
{
    game_behavior_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_behavior_table_lock_unlock_initialised)
{
    game_behavior_table value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_behavior_table_get_error_after_success)
{
    game_behavior_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_behavior_table_get_error_str_after_success)
{
    game_behavior_table value;
    const char *error_string;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    error_string = value.get_error_str();
    FT_ASSERT_NEQ(ft_nullptr, error_string);
    return (1);
}

FT_TEST(test_game_behavior_table_destroy_twice_is_safe)
{
    game_behavior_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_behavior_table_initialize_default_is_empty)
{
    game_behavior_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_profiles().size());
    return (1);
}

FT_TEST(test_game_behavior_table_get_error_after_registration)
{
    game_behavior_table table;
    game_behavior_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize());
    profile.set_profile_id(13);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}
