#include "../test_internal.hpp"

#include "../../Modules/Game/game_reputation.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_reputation_thread_safe_lifecycle)
{
    game_reputation reputation;
    ft_map<int32_t, int32_t> milestones;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, milestones.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize(milestones, 12));
    FT_ASSERT_EQ(12, reputation.get_total_rep());
    FT_ASSERT_EQ(0, reputation.get_current_rep());
    reputation.set_total_rep(25);
    reputation.set_current_rep(7);
    reputation.set_milestone(3, 9);
    reputation.set_rep(3, 11);
    FT_ASSERT_EQ(25, reputation.get_total_rep());
    FT_ASSERT_EQ(7, reputation.get_current_rep());
    FT_ASSERT_EQ(9, reputation.get_milestone(3));
    FT_ASSERT_EQ(11, reputation.get_rep(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, milestones.destroy());
    return (1);
}
