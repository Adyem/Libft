#include "../test_internal.hpp"

#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_state_thread_safe_lifecycle)
{
    game_state state;
    ft_string key;
    ft_string value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("mode"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("survival"));
    state.set_variable(key, value);
    FT_ASSERT(state.get_variable(key) != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(state.get_variable(key)->c_str(), "survival"));
    state.remove_variable(key);
    FT_ASSERT(state.get_variable(key) == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.destroy());
    return (1);
}
