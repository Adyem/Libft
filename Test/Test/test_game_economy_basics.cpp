#include "../test_internal.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_achievement_progress_completion, "Game: achievements track goal completion state")
{
    ft_achievement achievement;

    achievement.set_goal(1, 5);
    achievement.set_goal(2, 2);
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(1, 3);
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(1, 2);
    achievement.add_progress(2, 2);
    FT_ASSERT_EQ(true, achievement.is_goal_complete(1));
    FT_ASSERT_EQ(true, achievement.is_goal_complete(2));
    FT_ASSERT_EQ(true, achievement.is_complete());
    return (1);
}

FT_TEST(test_game_achievement_invalid_goal_id_sets_error, "Game: achievements reject negative goal identifiers")
{
    ft_achievement achievement;

    achievement.set_goal(-4, 3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_goal(-4));
    return (1);
}

FT_TEST(test_game_reputation_milestone_roundtrip, "Game: reputation milestone insertion and lookup")
{
    ft_reputation reputation;

    reputation.set_milestone(3, 50);
    FT_ASSERT_EQ(50, reputation.get_milestone(3));
    return (1);
}

FT_TEST(test_game_reputation_invalid_milestone_rejected, "Game: reputation rejects negative milestone ids")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(0, reputation.get_milestone(-2));
    return (1);
}

FT_TEST(test_game_currency_rate_copy_matches_source, "Game: currency rate copy preserves values")
{
    ft_currency_rate original;
    ft_currency_rate clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(5, 2.5, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(original));

    FT_ASSERT_EQ(5, clone.get_currency_id());
    FT_ASSERT_EQ(2.5, clone.get_rate_to_base());
    FT_ASSERT_EQ(4, clone.get_display_precision());
    FT_ASSERT_EQ(5, original.get_currency_id());
    FT_ASSERT_EQ(2.5, original.get_rate_to_base());
    FT_ASSERT_EQ(4, original.get_display_precision());
    return (1);
}
