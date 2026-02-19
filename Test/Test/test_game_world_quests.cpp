#include "../test_internal.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_item.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_quest_basic_fields_reflect_updates, "quest setters update the stored values")
{
    ft_quest quest;

    quest.set_id(42);
    quest.set_phases(3);
    quest.set_current_phase(1);
    quest.set_description("Rescue the guild");
    quest.set_objective("Find the hidden key");
    quest.set_reward_experience(250);

    FT_ASSERT_EQ(42, quest.get_id());
    FT_ASSERT_EQ(3, quest.get_phases());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(ft_string("Rescue the guild"), quest.get_description());
    FT_ASSERT_EQ(ft_string("Find the hidden key"), quest.get_objective());
    FT_ASSERT_EQ(250, quest.get_reward_experience());
    return (1);
}

FT_TEST(test_game_quest_progress_advances_until_complete, "advancing phases reaches completion state")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(0);
    quest.advance_phase();
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(false, quest.is_complete());

    quest.advance_phase();
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(true, quest.is_complete());
    return (1);
}

FT_TEST(test_game_quest_reduce_phases_clamps_current, "reducing phases clamps the current phase")
{
    ft_quest quest;

    quest.set_phases(5);
    quest.set_current_phase(4);
    quest.set_phases(2);
    FT_ASSERT_EQ(2, quest.get_phases());
    FT_ASSERT_EQ(2, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_reward_items_accept_valid_entries, "reward items list accepts non-null pointers")
{
    ft_quest quest;
    ft_vector<ft_sharedptr<ft_item>> rewards;
    ft_sharedptr<ft_item> potion(new (std::nothrow) ft_item());

    FT_ASSERT_EQ(true, potion.get() != ft_nullptr);
    potion->set_item_id(7);
    potion->set_stack_size(1);
    rewards.push_back(potion);
    quest.set_reward_items(rewards);
    FT_ASSERT_EQ(1u, quest.get_reward_items().size());
    FT_ASSERT_EQ(7, quest.get_reward_items()[0]->get_item_id());
    return (1);
}
