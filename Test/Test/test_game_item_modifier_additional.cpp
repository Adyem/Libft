#include "../test_internal.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_game_item_modifier_initialize_defaults)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(0, modifier.get_id());
    FT_ASSERT_EQ(0, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_initialize_values)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(-4, 19));
    FT_ASSERT_EQ(-4, modifier.get_id());
    FT_ASSERT_EQ(19, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_set_id_positive)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    modifier.set_id(12);
    FT_ASSERT_EQ(12, modifier.get_id());
    return (1);
}

FT_TEST(test_game_item_modifier_set_id_negative)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    modifier.set_id(-12);
    FT_ASSERT_EQ(-12, modifier.get_id());
    return (1);
}

FT_TEST(test_game_item_modifier_set_value_positive)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    modifier.set_value(27);
    FT_ASSERT_EQ(27, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_set_value_negative)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    modifier.set_value(-27);
    FT_ASSERT_EQ(-27, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_copy_preserves_values)
{
    game_item_modifier source;
    game_item_modifier copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(8, -3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(8, copy.get_id());
    FT_ASSERT_EQ(-3, copy.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_move_clears_source)
{
    game_item_modifier source;
    game_item_modifier destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(8, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_item_modifier &&>(source)));
    FT_ASSERT_EQ(8, destination.get_id());
    FT_ASSERT_EQ(3, destination.get_value());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_item_modifier_move_method)
{
    game_item_modifier source;
    game_item_modifier destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(2, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(2, destination.get_id());
    FT_ASSERT_EQ(5, destination.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_self_move)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(2, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.move(modifier));
    FT_ASSERT_EQ(2, modifier.get_id());
    FT_ASSERT_EQ(5, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_destroy_resets_values)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(2, 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    FT_ASSERT_EQ(0, modifier._id);
    FT_ASSERT_EQ(0, modifier._value);
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, modifier._initialised_state);
    return (1);
}

FT_TEST(test_game_item_modifier_destroy_is_idempotent)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    return (1);
}

FT_TEST(test_game_item_modifier_thread_safety_enable)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, modifier.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_thread_safety_disable)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, modifier.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_thread_safety_reenable)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, modifier.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_thread_safe_setters)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.enable_thread_safety());
    modifier.set_id(41);
    modifier.set_value(42);
    FT_ASSERT_EQ(41, modifier.get_id());
    FT_ASSERT_EQ(42, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_copy_after_mutation)
{
    game_item_modifier source;
    game_item_modifier copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(1, 2));
    source.set_id(3);
    source.set_value(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(3, copy.get_id());
    FT_ASSERT_EQ(4, copy.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_error_after_valid_operation)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize());
    modifier.set_id(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.get_error());
    FT_ASSERT_STR_EQ("Success", modifier.get_error_str());
    return (1);
}

FT_TEST(test_game_item_modifier_extreme_values)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(INT32_MIN, INT32_MAX));
    FT_ASSERT_EQ(INT32_MIN, modifier.get_id());
    FT_ASSERT_EQ(INT32_MAX, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_reinitialize_after_destroy)
{
    game_item_modifier modifier;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(1, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(3, 4));
    FT_ASSERT_EQ(3, modifier.get_id());
    FT_ASSERT_EQ(4, modifier.get_value());
    return (1);
}

