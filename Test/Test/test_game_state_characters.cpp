#include "../test_internal.hpp"
#include "../../Game/game_state.hpp"
#include "../../Game/game_character.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_state_adds_valid_character)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    ft_sharedptr<game_character> hero(new (std::nothrow) game_character());
    ft_vector<ft_sharedptr<game_character> > *characters;
    int result;

    FT_ASSERT(static_cast<bool>(hero));
    result = state.add_character(hero);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    characters = &state.get_characters();
    FT_ASSERT_EQ(1, characters->size());
    FT_ASSERT((*characters)[0] == hero);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    return (1);
}

FT_TEST(test_game_state_rejects_null_character)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    ft_sharedptr<game_character> empty;
    int result;

    result = state.add_character(empty);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, result);
    FT_ASSERT_EQ(0, state.get_characters().size());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    return (1);
}

FT_TEST(test_game_state_remove_invalid_index_sets_error)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    ft_sharedptr<game_character> hero(new (std::nothrow) game_character());

    FT_ASSERT(static_cast<bool>(hero));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.add_character(hero));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    state.remove_character(5);
    FT_ASSERT_EQ(1, state.get_characters().size());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    return (1);
}

FT_TEST(test_game_state_remove_character_shrinks_vector)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    ft_sharedptr<game_character> first(new (std::nothrow) game_character());
    ft_sharedptr<game_character> second(new (std::nothrow) game_character());

    FT_ASSERT(static_cast<bool>(first));
    FT_ASSERT(static_cast<bool>(second));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.add_character(first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.add_character(second));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    state.remove_character(0);
    FT_ASSERT_EQ(1, state.get_characters().size());
    FT_ASSERT(state.get_characters()[0] == second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    return (1);
}

FT_TEST(test_game_state_thread_safety_toggle)
{
    game_state state;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());

    FT_ASSERT_EQ(false, state.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(true, state.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    FT_ASSERT_EQ(false, state.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.get_error());
    return (1);
}
