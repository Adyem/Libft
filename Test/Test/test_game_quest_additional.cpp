#include "../test_internal.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Errno/errno.hpp"

static void make_quest_string(ft_string &value, const char *text)
{
    (void)value.initialize(text);
    return ;
}

FT_TEST(test_game_quest_default_fields)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(0, quest.get_id());
    FT_ASSERT_EQ(0, quest.get_phases());
    FT_ASSERT_EQ(0, quest.get_current_phase());
    FT_ASSERT_EQ(0, quest.get_reward_experience());
    return (1);
}

FT_TEST(test_game_quest_set_id_negative)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_id(-8);
    FT_ASSERT_EQ(0, quest.get_id());
    return (1);
}

FT_TEST(test_game_quest_set_phases_positive)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(5);
    FT_ASSERT_EQ(5, quest.get_phases());
    return (1);
}

FT_TEST(test_game_quest_set_phases_zero)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(0);
    FT_ASSERT_EQ(0, quest.get_phases());
    return (1);
}

FT_TEST(test_game_quest_set_current_phase_valid)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(4);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(3, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_set_current_phase_rejects_negative)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_current_phase(-1);
    FT_ASSERT_EQ(0, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_set_current_phase_rejects_above_phases)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(2);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(0, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_description_round_trip)
{
    game_quest quest;
    ft_string description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    make_quest_string(description, "description");
    quest.set_description(description);
    FT_ASSERT_STR_EQ("description", quest.get_description().c_str());
    return (1);
}

FT_TEST(test_game_quest_objective_round_trip)
{
    game_quest quest;
    ft_string objective;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    make_quest_string(objective, "objective");
    quest.set_objective(objective);
    FT_ASSERT_STR_EQ("objective", quest.get_objective().c_str());
    return (1);
}

FT_TEST(test_game_quest_reward_experience_positive)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_reward_experience(120);
    FT_ASSERT_EQ(120, quest.get_reward_experience());
    return (1);
}

FT_TEST(test_game_quest_reward_experience_rejects_negative)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_reward_experience(-1);
    FT_ASSERT_EQ(0, quest.get_reward_experience());
    return (1);
}

FT_TEST(test_game_quest_complete_when_zero_phases)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_TRUE, quest.is_complete());
    return (1);
}

FT_TEST(test_game_quest_not_complete_before_final_phase)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(2);
    FT_ASSERT_EQ(FT_FALSE, quest.is_complete());
    quest.advance_phase();
    FT_ASSERT_EQ(FT_FALSE, quest.is_complete());
    return (1);
}

FT_TEST(test_game_quest_complete_after_all_phases)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(2);
    quest.advance_phase();
    quest.advance_phase();
    FT_ASSERT_EQ(FT_TRUE, quest.is_complete());
    return (1);
}

FT_TEST(test_game_quest_advance_does_not_overflow_phases)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_phases(1);
    quest.advance_phase();
    quest.advance_phase();
    FT_ASSERT_EQ(1, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_reward_items_empty_after_initialize)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), quest.get_reward_items().size());
    return (1);
}

FT_TEST(test_game_quest_reward_items_round_trip)
{
    game_quest quest;
    ft_vector<ft_sharedptr<game_item> > items;
    ft_sharedptr<game_item> item(new game_item());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, items.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, items.push_back(item));
    quest.set_reward_items(items);
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), quest.get_reward_items().size());
    return (1);
}

FT_TEST(test_game_quest_thread_safety)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, quest.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_quest_copy_preserves_fields)
{
    game_quest source;
    game_quest copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_id(5);
    source.set_phases(3);
    source.set_current_phase(1);
    source.set_reward_experience(99);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(5, copy.get_id());
    FT_ASSERT_EQ(3, copy.get_phases());
    FT_ASSERT_EQ(1, copy.get_current_phase());
    FT_ASSERT_EQ(99, copy.get_reward_experience());
    return (1);
}

FT_TEST(test_game_quest_move_transfers_fields)
{
    game_quest source;
    game_quest destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_id(7);
    source.set_phases(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(7, destination.get_id());
    FT_ASSERT_EQ(2, destination.get_phases());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_quest_destroy_resets_fields)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    quest.set_id(3);
    quest.set_phases(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.destroy());
    FT_ASSERT_EQ(0, quest._id);
    FT_ASSERT_EQ(0, quest._phases);
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, quest._initialised_state);
    return (1);
}
