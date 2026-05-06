#include "../test_internal.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_quest_basic_fields_reflect_updates)
{
    game_quest quest;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_id(42);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_phases(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_current_phase(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    ft_string description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("Rescue the guild"));
    quest.set_description(description);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    ft_string objective;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, objective.initialize("Find the hidden key"));
    quest.set_objective(objective);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_reward_experience(250);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    FT_ASSERT_EQ(42, quest.get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(3, quest.get_phases());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_STR_EQ("Rescue the guild", quest.get_description().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_STR_EQ("Find the hidden key", quest.get_objective().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(250, quest.get_reward_experience());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_quest_progress_advances_until_complete)
{
    game_quest quest;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_phases(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_current_phase(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(false, quest.is_complete());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(true, quest.is_complete());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_quest_reduce_phases_clamps_current)
{
    game_quest quest;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_phases(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_current_phase(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.set_phases(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(2, quest.get_phases());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_quest_reward_items_accept_valid_entries)
{
    game_quest quest;
    ft_vector<ft_sharedptr<game_item>> rewards;
    ft_sharedptr<game_item> potion(new (std::nothrow) game_item());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rewards.initialize());
    FT_ASSERT_EQ(true, potion.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, potion->initialize());
    potion->set_item_id(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, potion->get_error());
    potion->set_stack_size(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, potion->get_error());
    rewards.push_back(potion);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, rewards.get_error());
    quest.set_reward_items(rewards);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(1u, quest.get_reward_items().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    FT_ASSERT_EQ(7, quest.get_reward_items()[0]->get_item_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}
