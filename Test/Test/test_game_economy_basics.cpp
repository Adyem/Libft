#include "../test_internal.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_achievement_progress_completion, "Game: achievements track goal completion state")
{
    ft_achievement achievement;

    achievement.set_goal(1, 5);
    achievement.set_goal(2, 2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(1, 3);
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(1, 2);
    achievement.add_progress(2, 2);
    FT_ASSERT_EQ(true, achievement.is_goal_complete(1));
    FT_ASSERT_EQ(true, achievement.is_goal_complete(2));
    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_invalid_goal_id_sets_error, "Game: achievements reject negative goal identifiers")
{
    ft_achievement achievement;

    achievement.set_goal(-4, 3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_game_reputation_milestone_roundtrip, "Game: reputation milestone insertion and lookup")
{
    ft_reputation reputation;

    reputation.set_milestone(3, 50);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    FT_ASSERT_EQ(50, reputation.get_milestone(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_invalid_milestone_rejected, "Game: reputation rejects negative milestone ids")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(0, reputation.get_milestone(-2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, reputation.get_error());
    return (1);
}

FT_TEST(test_game_currency_rate_move_transfers_values, "Game: currency rate move clears source")
{
    ft_currency_rate original(5, 2.5, 4);
    ft_currency_rate moved(ft_move(original));

    FT_ASSERT_EQ(5, moved.get_currency_id());
    FT_ASSERT_EQ(2.5, moved.get_rate_to_base());
    FT_ASSERT_EQ(4, moved.get_display_precision());
    FT_ASSERT_EQ(0, original.get_currency_id());
    FT_ASSERT_EQ(0.0, original.get_rate_to_base());
    FT_ASSERT_EQ(0, original.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    return (1);
}
