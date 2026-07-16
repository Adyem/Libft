#include "../test_internal.hpp"

#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_behavior_profile_thread_safe_lifecycle)
{
    game_behavior_profile profile;
    ft_vector<game_behavior_action> actions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(11, 0.8, 0.2, actions));
    FT_ASSERT_EQ(11, profile.get_profile_id());
    FT_ASSERT_EQ(0.8, profile.get_aggression_weight());
    FT_ASSERT_EQ(0.2, profile.get_caution_weight());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), profile.get_actions().size());
    profile.set_profile_id(18);
    profile.set_aggression_weight(1.1);
    profile.set_caution_weight(0.4);
    FT_ASSERT_EQ(18, profile.get_profile_id());
    FT_ASSERT_EQ(1.1, profile.get_aggression_weight());
    FT_ASSERT_EQ(0.4, profile.get_caution_weight());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    return (1);
}
