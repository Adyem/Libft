#include "../../Game/game_state.hpp"
#include "../../Game/game_character.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_state_adds_valid_character, "Game: add_character stores valid entries")
{
    ft_game_state state;
    ft_sharedptr<ft_character> hero(new (std::nothrow) ft_character());
    ft_vector<ft_sharedptr<ft_character> > *characters;
    int result;

    FT_ASSERT(static_cast<bool>(hero));
    result = state.add_character(hero);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, result);
    characters = &state.get_characters();
    FT_ASSERT_EQ(1, characters->size());
    FT_ASSERT((*characters)[0] == hero);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, state.get_error());
    return (1);
}

FT_TEST(test_game_state_rejects_null_character, "Game: add_character rejects null shared pointers")
{
    ft_game_state state;
    ft_sharedptr<ft_character> empty;
    int result;

    result = state.add_character(empty);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, result);
    FT_ASSERT_EQ(0, state.get_characters().size());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    return (1);
}

FT_TEST(test_game_state_propagates_character_error, "Game: add_character surfaces character errors")
{
    ft_game_state state;
    ft_sharedptr<ft_character> hero(new (std::nothrow) ft_character());
    int result;

    FT_ASSERT(static_cast<bool>(hero));
    hero.set_error(FT_ERR_GAME_GENERAL_ERROR);
    result = state.add_character(hero);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, result);
    FT_ASSERT_EQ(0, state.get_characters().size());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    return (1);
}

FT_TEST(test_game_state_remove_invalid_index_sets_error, "Game: remove_character guards out of range indices")
{
    ft_game_state state;
    ft_sharedptr<ft_character> hero(new (std::nothrow) ft_character());

    FT_ASSERT(static_cast<bool>(hero));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, state.add_character(hero));
    state.remove_character(5);
    FT_ASSERT_EQ(1, state.get_characters().size());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    return (1);
}

FT_TEST(test_game_state_remove_character_shrinks_vector, "Game: remove_character deletes stored character")
{
    ft_game_state state;
    ft_sharedptr<ft_character> first(new (std::nothrow) ft_character());
    ft_sharedptr<ft_character> second(new (std::nothrow) ft_character());

    FT_ASSERT(static_cast<bool>(first));
    FT_ASSERT(static_cast<bool>(second));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, state.add_character(first));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, state.add_character(second));
    state.remove_character(0);
    FT_ASSERT_EQ(1, state.get_characters().size());
    FT_ASSERT(state.get_characters()[0] == second);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, state.get_error());
    return (1);
}
