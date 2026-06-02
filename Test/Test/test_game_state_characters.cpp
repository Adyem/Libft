#include "../test_internal.hpp"
#include "../../Modules/Game/game_state.hpp"
#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/CMA/CMA.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/vector.hpp"
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
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    FT_ASSERT_EQ(0, state.get_characters().size());
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
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, state.get_error());
    FT_ASSERT_EQ(1, state.get_characters().size());
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

FT_TEST(test_game_state_move_failure_destroys_destination)
{
    game_state source;
    game_state destination;
    ft_size_t world_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    world_index = 0;
    while (world_index < 8)
    {
        ft_sharedptr<game_world> world_pointer(new (std::nothrow) game_world());

        FT_ASSERT(world_pointer.get() != ft_nullptr);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, world_pointer->initialize());
        source.get_worlds().push_back(world_pointer);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_worlds().get_error());
        world_index += 1;
    }
    FT_ASSERT_EQ(static_cast<ft_size_t>(9), source.get_worlds().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(1));
    FT_ASSERT_NE(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, destination._initialised_state);
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_state_add_character_allocation_failure_reports_error)
{
    game_state state;
    ft_sharedptr<game_character> characters[9];
    ft_size_t character_index;
    int32_t add_result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    character_index = 0;
    while (character_index < 9)
    {
        characters[character_index] = ft_sharedptr<game_character>(new (std::nothrow) game_character());
        FT_ASSERT(characters[character_index].get() != ft_nullptr);
        character_index += 1;
    }
    character_index = 0;
    while (character_index < 8)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, state.add_character(characters[character_index]));
        character_index += 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(1));
    add_result = state.add_character(characters[8]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_NE(FT_ERR_SUCCESS, add_result);
    FT_ASSERT_EQ(static_cast<ft_size_t>(8), state.get_characters().size());
    return (1);
}

FT_TEST(test_game_state_set_variable_insert_failure_reports_error)
{
    game_state state;
    ft_string key;
    ft_string value;
    int32_t state_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, state.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, key.initialize("quest_stage_long_key"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("active_stage_long_value"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(1));
    state.set_variable(key, value);
    state_error = state.get_error();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cma_set_alloc_limit(0));
    FT_ASSERT_NE(FT_ERR_SUCCESS, state_error);
    FT_ASSERT(state.get_variable(key) == ft_nullptr);
    return (1);
}
