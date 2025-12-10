#include "../../Game/game_skill.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_game_skill_rejects_negative_id, "Game: skill set_id rejects negative values")
{
    ft_skill skill;

    skill.set_id(-2);

    FT_ASSERT_EQ(0, skill.get_id());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, skill.get_error());
    return (1);
}

FT_TEST(test_game_skill_modifier_accumulation, "Game: skill modifier setters accumulate deltas")
{
    ft_skill skill;

    skill.set_modifier1(3);
    skill.add_modifier1(4);
    skill.sub_modifier1(2);

    FT_ASSERT_EQ(5, skill.get_modifier1());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, skill.get_error());
    return (1);
}

FT_TEST(test_game_upgrade_add_level_caps_at_max, "Game: upgrade add_level clamps to maximum")
{
    ft_upgrade upgrade;

    upgrade.set_max_level(5);
    upgrade.set_current_level(2);
    upgrade.add_level(10);

    FT_ASSERT_EQ(5, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, upgrade.get_error());
    return (1);
}

FT_TEST(test_game_upgrade_sub_level_floors_at_zero, "Game: upgrade sub_level floors progress at zero")
{
    ft_upgrade upgrade;

    upgrade.set_current_level(3);
    upgrade.sub_level(7);

    FT_ASSERT_EQ(0, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, upgrade.get_error());
    return (1);
}

FT_TEST(test_game_quest_advance_without_phases_sets_error, "Game: quest advance_phase rejects zero phases")
{
    ft_quest quest;

    quest.set_phases(0);
    quest.advance_phase();

    FT_ASSERT_EQ(0, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    return (1);
}
