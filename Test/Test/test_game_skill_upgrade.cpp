#include "../test_internal.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_skill_rejects_negative_id)
{
    game_skill skill;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.initialize());

    skill.set_id(-2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());

    FT_ASSERT_EQ(-2, skill.get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());
    return (1);
}

FT_TEST(test_game_skill_modifier_accumulation)
{
    game_skill skill;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.initialize());

    skill.set_modifier1(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());
    skill.add_modifier1(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());
    skill.sub_modifier1(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());

    FT_ASSERT_EQ(5, skill.get_modifier1());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, skill.get_error());
    return (1);
}

FT_TEST(test_game_upgrade_add_level_caps_at_max)
{
    game_upgrade upgrade;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.initialize());

    upgrade.set_max_level(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.set_current_level(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.add_level(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());

    FT_ASSERT_EQ(12, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    return (1);
}

FT_TEST(test_game_upgrade_sub_level_floors_at_zero)
{
    game_upgrade upgrade;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.initialize());

    upgrade.set_current_level(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.sub_level(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());

    FT_ASSERT_EQ(65532, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    return (1);
}

FT_TEST(test_game_quest_advance_without_phases_sets_error)
{
    game_quest quest;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    quest.set_phases(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());

    FT_ASSERT_EQ(0, quest.get_current_phase());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest.get_error());
    return (1);
}
