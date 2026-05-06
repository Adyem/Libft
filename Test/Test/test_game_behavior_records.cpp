#include "../test_internal.hpp"
#include "../../Modules/Game/game_behavior_action.hpp"
#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_behavior_action_initialize_copy_semantics)
{
    game_behavior_action original;
    game_behavior_action copy;
    game_behavior_action assigned;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(8, 0.75, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(8, copy.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.75, copy.get_weight());
    FT_ASSERT_DOUBLE_EQ(4.0, copy.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.get_error());
    original.set_weight(2.25);
    FT_ASSERT_EQ(8, assigned.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.75, assigned.get_weight());
    FT_ASSERT_DOUBLE_EQ(4.0, assigned.get_cooldown_seconds());
    FT_ASSERT_DOUBLE_EQ(2.25, original.get_weight());
    return (1);
}

FT_TEST(test_behavior_action_initialize_move_semantics)
{
    game_behavior_action source;
    game_behavior_action moved;
    game_behavior_action reassigned;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 0.30, 6.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(3, moved.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.30, moved.get_weight());
    FT_ASSERT_DOUBLE_EQ(6.5, moved.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(9, 1.10, 1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.get_error());
    FT_ASSERT_EQ(9, reassigned.get_action_id());
    FT_ASSERT_DOUBLE_EQ(1.10, reassigned.get_weight());
    FT_ASSERT_DOUBLE_EQ(1.5, reassigned.get_cooldown_seconds());
    FT_ASSERT_EQ(0, source.get_action_id());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_weight());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_cooldown_seconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}

static int assert_profile_values(const game_behavior_profile &profile, int id, double aggression,
        double caution, int first_action_id)
{
    FT_ASSERT_EQ(id, profile.get_profile_id());
    FT_ASSERT_DOUBLE_EQ(aggression, profile.get_aggression_weight());
    FT_ASSERT_DOUBLE_EQ(caution, profile.get_caution_weight());
    FT_ASSERT_EQ(first_action_id, profile.get_actions()[0].get_action_id());
    return (1);
}

FT_TEST(test_behavior_profile_copy_semantics)
{
    ft_vector<game_behavior_action> actions;
    game_behavior_profile original;
    game_behavior_profile copy;
    game_behavior_profile assigned;
    game_behavior_action action_entry_first;
    game_behavior_action action_entry_second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.initialize(4, 0.4, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.get_error());
    actions.push_back(action_entry_first);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.initialize(6, 0.6, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.get_error());
    actions.push_back(action_entry_second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(12, 0.7, 0.3, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    assert_profile_values(copy, 12, 0.7, 0.3, 4);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.initialize(original));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.get_error());
    original.get_actions()[0].set_action_id(99);
    assert_profile_values(assigned, 12, 0.7, 0.3, 4);
    assert_profile_values(original, 12, 0.7, 0.3, 99);
    return (1);
}

FT_TEST(test_behavior_profile_move_semantics)
{
    ft_vector<game_behavior_action> actions;
    game_behavior_profile source;
    game_behavior_profile moved;
    game_behavior_profile reassigned;
    game_behavior_action action_entry_first;
    game_behavior_action action_entry_second;
    game_behavior_action action_entry_third;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, actions.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.initialize(1, 0.2, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_first.get_error());
    actions.push_back(action_entry_first);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(30, 0.9, 0.1, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    assert_profile_values(moved, 30, 0.9, 0.1, 1);
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    actions.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.initialize(2, 0.3, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_second.get_error());
    actions.push_back(action_entry_second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_third.initialize(3, 0.7, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, action_entry_third.get_error());
    actions.push_back(action_entry_third);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(44, 0.55, 0.45, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.initialize(ft_move(source)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reassigned.get_error());
    assert_profile_values(reassigned, 44, 0.55, 0.45, 2);
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}
