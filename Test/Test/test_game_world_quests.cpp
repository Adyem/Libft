#include "../../Game/game_world.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_item.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_quest_advance_progress_to_completion, "Game quest advances phases until completion")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(0);
    quest.advance_phase();
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(false, quest.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());

    quest.advance_phase();
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(true, quest.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}


FT_TEST(test_game_quest_invalid_and_overflow_phases_report_errors, "Game quest validates phase advancement bounds")
{
    ft_quest quest;

    quest.set_phases(0);
    quest.set_current_phase(1);
    ft_errno = ER_SUCCESS;
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(0, quest.get_current_phase());

    quest.set_phases(1);
    quest.set_current_phase(1);
    ft_errno = ER_SUCCESS;
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, quest.get_error());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    return (1);
}


FT_TEST(test_game_quest_rejects_out_of_range_phase, "Game quest blocks setting invalid current phase")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(0, quest.get_current_phase());
    return (1);
}


FT_TEST(test_game_quest_clamps_current_phase_when_phases_shrink, "Game quest clamps current phase when reducing total phases")
{
    ft_quest quest;

    quest.set_phases(4);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    quest.set_phases(2);
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}


FT_TEST(test_game_quest_copy_preserves_configuration, "Game quest copy constructor keeps configured fields")
{
    ft_quest quest;

    quest.set_id(8);
    quest.set_phases(3);
    quest.set_current_phase(1);
    quest.set_description("Find the relic");
    quest.set_objective("Reach the temple");
    quest.set_reward_experience(50);

    ft_quest copy(quest);
    FT_ASSERT_EQ(8, copy.get_id());
    FT_ASSERT_EQ(3, copy.get_phases());
    FT_ASSERT_EQ(1, copy.get_current_phase());
    FT_ASSERT_EQ(50, copy.get_reward_experience());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    return (1);
}


FT_TEST(test_game_quest_clears_error_after_valid_phase_set, "Game quest resets error when setting a valid phase")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    quest.set_current_phase(1);
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}


FT_TEST(test_game_quest_rejects_negative_phase_counts, "Game quest retains previous phases when rejecting negatives")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(1);
    quest.set_phases(-1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(2, quest.get_phases());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    return (1);
}


FT_TEST(test_game_quest_move_assignment_resets_source, "Game quest move assignment moves data and clears origin")
{
    ft_quest source;
    ft_quest destination;

    source.set_id(15);
    source.set_phases(5);
    source.set_current_phase(2);
    source.set_description("Recover the crown");
    source.set_objective("Reach the throne room");
    source.set_reward_experience(120);
    destination.set_id(3);
    destination.set_phases(1);
    destination.set_current_phase(1);

    destination = ft_move(source);
    FT_ASSERT_EQ(15, destination.get_id());
    FT_ASSERT_EQ(5, destination.get_phases());
    FT_ASSERT_EQ(2, destination.get_current_phase());
    FT_ASSERT_EQ(ft_string("Recover the crown"), destination.get_description());
    FT_ASSERT_EQ(ft_string("Reach the throne room"), destination.get_objective());
    FT_ASSERT_EQ(120, destination.get_reward_experience());
    FT_ASSERT_EQ(0, source.get_id());
    FT_ASSERT_EQ(0, source.get_phases());
    FT_ASSERT_EQ(0, source.get_current_phase());
    FT_ASSERT_EQ(ft_string(""), source.get_description());
    FT_ASSERT_EQ(ft_string(""), source.get_objective());
    FT_ASSERT_EQ(0, source.get_reward_experience());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_get_quest_propagates_reward_experience_error, "Game world getter reflects invalid quest experience")
{
    ft_world world;
    ft_sharedptr<ft_quest> quest = world.get_quest();

    ft_errno = ER_SUCCESS;
    quest->set_reward_experience(-10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest->get_error());
    world.get_quest();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}


FT_TEST(test_game_quest_set_reward_items_rejects_null_entry, "Game quest refuses null reward items")
{
    ft_quest quest;
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> null_item;

    rewards.push_back(null_item);
    FT_ASSERT_EQ(ER_SUCCESS, rewards.get_error());
    ft_errno = ER_SUCCESS;
    quest.set_reward_items(rewards);
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, quest.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    FT_ASSERT_EQ((size_t)0, quest.get_reward_items().size());
    return (1);
}


FT_TEST(test_game_world_get_quest_propagates_reward_item_error, "Game world getter mirrors quest reward item failure")
{
    ft_world world;
    ft_sharedptr<ft_quest> quest = world.get_quest();
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> null_item;

    rewards.push_back(null_item);
    quest->set_reward_items(rewards);
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, quest->get_error());
    ft_errno = ER_SUCCESS;
    world.get_quest();
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_errno);
    return (1);
}


FT_TEST(test_game_world_copy_preserves_quest_reward_items, "Game world copy retains quest reward item list")
{
    ft_world world;
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> first_item(new ft_item());
    ft_sharedptr<ft_item> second_item(new ft_item());

    first_item->set_item_id(301);
    second_item->set_item_id(302);
    rewards.push_back(first_item);
    rewards.push_back(second_item);
    world.get_quest()->set_reward_items(rewards);

    ft_world copy(world);
    FT_ASSERT_EQ((size_t)2, copy.get_quest()->get_reward_items().size());
    FT_ASSERT_EQ(301, copy.get_quest()->get_reward_items()[0]->get_item_id());
    FT_ASSERT_EQ(302, copy.get_quest()->get_reward_items()[1]->get_item_id());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_quest_reward_items, "Game world move transfers quest reward item ownership")
{
    ft_world source;
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> reward_item(new ft_item());

    reward_item->set_item_id(404);
    rewards.push_back(reward_item);
    source.get_quest()->set_reward_items(rewards);

    ft_world moved(ft_move(source));
    FT_ASSERT_EQ((size_t)1, moved.get_quest()->get_reward_items().size());
    FT_ASSERT_EQ(404, moved.get_quest()->get_reward_items()[0]->get_item_id());
    source.get_quest();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_quest_reward_items_recovery_clears_error, "Game world clears quest reward item errors after replacement")
{
    ft_world world;
    ft_sharedptr<ft_quest> quest = world.get_quest();
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> null_item;

    rewards.push_back(null_item);
    quest->set_reward_items(rewards);
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, quest->get_error());

    ft_vector<ft_sharedptr<ft_item> > valid_rewards;
    ft_sharedptr<ft_item> valid_item(new ft_item());

    valid_item->set_item_id(505);
    valid_rewards.push_back(valid_item);
    quest->set_reward_items(valid_rewards);
    world.get_quest();
    FT_ASSERT_EQ((size_t)1, quest->get_reward_items().size());
    FT_ASSERT_EQ(505, quest->get_reward_items()[0]->get_item_id());
    FT_ASSERT_EQ(ER_SUCCESS, quest->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_assignment_preserves_quest_rewards, "Game world copy assignment retains quest reward items")
{
    ft_world source;
    ft_world target;
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> reward_item(new ft_item());

    reward_item->set_item_id(606);
    rewards.push_back(reward_item);
    source.get_quest()->set_reward_items(rewards);

    target = source;
    FT_ASSERT_EQ((size_t)1, target.get_quest()->get_reward_items().size());
    FT_ASSERT_EQ(606, target.get_quest()->get_reward_items()[0]->get_item_id());
    FT_ASSERT_EQ(ER_SUCCESS, target.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_move_assignment_transfers_quest_rewards, "Game world move assignment transfers quest reward ownership")
{
    ft_world source;
    ft_world target;
    ft_vector<ft_sharedptr<ft_item> > rewards;
    ft_sharedptr<ft_item> reward_item(new ft_item());

    reward_item->set_item_id(707);
    rewards.push_back(reward_item);
    source.get_quest()->set_reward_items(rewards);

    target = ft_move(source);
    FT_ASSERT_EQ((size_t)1, target.get_quest()->get_reward_items().size());
    FT_ASSERT_EQ(707, target.get_quest()->get_reward_items()[0]->get_item_id());
    source.get_quest();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, target.get_error());
    return (1);
}


