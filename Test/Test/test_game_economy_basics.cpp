#include "../test_internal.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_reputation.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_achievement_progress_completion)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());

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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_invalid_goal_id_sets_error)
{
    game_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, achievement.initialize());

    achievement.set_goal(-4, 3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_goal(-4));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    return (1);
}

FT_TEST(test_game_reputation_milestone_roundtrip)
{
    game_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());

    reputation.set_milestone(3, 50);
    FT_ASSERT_EQ(50, reputation.get_milestone(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.get_error());
    return (1);
}

FT_TEST(test_game_reputation_invalid_milestone_rejected)
{
    game_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reputation.initialize());

    FT_ASSERT_EQ(0, reputation.get_milestone(-2));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, reputation.get_error());
    return (1);
}

FT_TEST(test_game_currency_rate_copy_matches_source)
{
    game_currency_rate original;
    game_currency_rate clone;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize(5, 2.5, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.initialize(original));

    FT_ASSERT_EQ(5, clone.get_currency_id());
    FT_ASSERT_EQ(2.5, clone.get_rate_to_base());
    FT_ASSERT_EQ(4, clone.get_display_precision());
    FT_ASSERT_EQ(5, original.get_currency_id());
    FT_ASSERT_EQ(2.5, original.get_rate_to_base());
    FT_ASSERT_EQ(4, original.get_display_precision());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, clone.get_error());
    return (1);
}
