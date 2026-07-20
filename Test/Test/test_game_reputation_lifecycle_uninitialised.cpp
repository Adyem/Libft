#include "../../Modules/Game/game_reputation.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_reputation_get_total_rep(game_reputation &value)
{
    (void)value.get_total_rep();
    return ;
}
static void game_reputation_set_total_rep(game_reputation &value)
{
    value.set_total_rep(1);
    return ;
}
static void game_reputation_add_total_rep(game_reputation &value)
{
    value.add_total_rep(1);
    return ;
}
static void game_reputation_sub_total_rep(game_reputation &value)
{
    value.sub_total_rep(1);
    return ;
}
static void game_reputation_get_current_rep(game_reputation &value)
{
    (void)value.get_current_rep();
    return ;
}
static void game_reputation_set_current_rep(game_reputation &value)
{
    value.set_current_rep(1);
    return ;
}
static void game_reputation_add_current_rep(game_reputation &value)
{
    value.add_current_rep(1);
    return ;
}
static void game_reputation_sub_current_rep(game_reputation &value)
{
    value.sub_current_rep(1);
    return ;
}
static void game_reputation_get_milestone(game_reputation &value)
{
    (void)value.get_milestone(1);
    return ;
}
static void game_reputation_set_milestone(game_reputation &value)
{
    value.set_milestone(1, 1);
    return ;
}

FT_TEST(test_new_game_reputation_get_total_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_get_total_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_set_total_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_set_total_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_add_total_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_add_total_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_sub_total_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_sub_total_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_get_current_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_get_current_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_set_current_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_set_current_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_add_current_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_add_current_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_sub_current_rep_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_sub_current_rep));
    return (1);
}
FT_TEST(test_new_game_reputation_get_milestone_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_get_milestone));
    return (1);
}
FT_TEST(test_new_game_reputation_set_milestone_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_reputation>(
                        game_reputation_set_milestone));
    return (1);
}
