#include "../test_internal.hpp"

#include "../../Modules/Game/game_behavior_table.hpp"
#include "../../Modules/Game/game_behavior_profile.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_behavior_table_thread_safe_lifecycle)
{
    game_behavior_table table;
    game_behavior_profile profile;
    game_behavior_profile fetched_profile;
    ft_vector<game_behavior_action> actions;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT(table.is_thread_safe() == FT_TRUE);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), table.get_profiles().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.initialize(12, 0.75, 0.25, actions));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.register_profile(profile));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), table.get_profiles().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.fetch_profile(12, fetched_profile));
    FT_ASSERT_EQ(12, fetched_profile.get_profile_id());
    FT_ASSERT_EQ(0.75, fetched_profile.get_aggression_weight());
    FT_ASSERT_EQ(0.25, fetched_profile.get_caution_weight());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), fetched_profile.get_actions().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, fetched_profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.destroy());
    return (1);
}
