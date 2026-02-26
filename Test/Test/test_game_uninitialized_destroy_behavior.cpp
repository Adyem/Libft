#include "../test_internal.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_rarity_band.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../Game/game_behavior_profile.hpp"
#include "../../Game/game_behavior_table.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_hooks.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Game/game_map3d.hpp"
#include "../../Game/game_progress_tracker.hpp"
#include "../../Game/game_skill.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/ft_dialogue_script.hpp"
#include "../../Game/game_data_catalog.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Game/game_pathfinding.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <csetjmp>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static volatile sig_atomic_t g_signal_caught = 0;
static sigjmp_buf g_signal_jump_buffer;

static void uninitialized_destructor_signal_handler(int signal_value)
{
    g_signal_caught = signal_value;
    siglongjmp(g_signal_jump_buffer, 1);
}

template <typename TypeName>
static int expect_no_sigabrt_on_uninitialized_destructor()
{
    struct sigaction old_action_abort;
    struct sigaction new_action_abort;
    struct sigaction old_action_iot;
    struct sigaction new_action_iot;
    int jump_result;

    std::memset(&new_action_abort, 0, sizeof(new_action_abort));
    std::memset(&new_action_iot, 0, sizeof(new_action_iot));
    new_action_abort.sa_handler = &uninitialized_destructor_signal_handler;
    new_action_iot.sa_handler = &uninitialized_destructor_signal_handler;
    sigemptyset(&new_action_abort.sa_mask);
    sigemptyset(&new_action_iot.sa_mask);
    if (sigaction(SIGABRT, &new_action_abort, &old_action_abort) != 0)
        return (0);
    if (sigaction(SIGIOT, &new_action_iot, &old_action_iot) != 0)
    {
        (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
        return (0);
    }

    g_signal_caught = 0;
    jump_result = sigsetjmp(g_signal_jump_buffer, 1);
    if (jump_result == 0)
    {
        TypeName object_instance;

        (void)object_instance;
    }

    (void)sigaction(SIGABRT, &old_action_abort, ft_nullptr);
    (void)sigaction(SIGIOT, &old_action_iot, ft_nullptr);
    return (g_signal_caught == 0);
}

static int is_non_aborting_uninitialized_destroy_result(int error_code)
{
    if (error_code == FT_ERR_SUCCESS)
        return (1);
    return (error_code == FT_ERR_INVALID_STATE);
}

FT_TEST(test_game_price_definition_destroy_uninitialized_returns_invalid_state,
    "ft_price_definition destroy on uninitialized object is non-aborting")
{
    ft_price_definition definition;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, definition.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_price_definition>());
    return (1);
}

FT_TEST(test_game_rarity_band_destroy_uninitialized_returns_invalid_state,
    "ft_rarity_band destroy on uninitialized object is non-aborting")
{
    ft_rarity_band band;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, band.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_rarity_band>());
    return (1);
}

FT_TEST(test_game_vendor_profile_destroy_uninitialized_returns_invalid_state,
    "ft_vendor_profile destroy on uninitialized object is non-aborting")
{
    ft_vendor_profile vendor;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, vendor.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, vendor.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_vendor_profile>());
    return (1);
}

FT_TEST(test_game_currency_rate_destroy_uninitialized_returns_invalid_state,
    "ft_currency_rate destroy on uninitialized object is non-aborting")
{
    ft_currency_rate rate;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, rate.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_currency_rate>());
    return (1);
}

FT_TEST(test_game_economy_table_destroy_uninitialized_returns_invalid_state,
    "ft_economy_table destroy on uninitialized object is non-aborting")
{
    ft_economy_table table;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_economy_table>());
    return (1);
}

FT_TEST(test_game_behavior_profile_destroy_uninitialized_returns_invalid_state,
    "ft_behavior_profile destroy on uninitialized object is non-aborting")
{
    ft_behavior_profile profile;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, profile.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_behavior_profile>());
    return (1);
}

FT_TEST(test_game_behavior_table_destroy_uninitialized_returns_invalid_state,
    "ft_behavior_table destroy on uninitialized object is non-aborting")
{
    ft_behavior_table table;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, table.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_behavior_table>());
    return (1);
}

FT_TEST(test_game_buff_destroy_uninitialized_returns_invalid_state,
    "ft_buff destroy on uninitialized object is non-aborting")
{
    ft_buff buff;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, buff.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, buff.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_buff>());
    return (1);
}

FT_TEST(test_game_debuff_destroy_uninitialized_returns_invalid_state,
    "ft_debuff destroy on uninitialized object is non-aborting")
{
    ft_debuff debuff;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, debuff.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, debuff.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_debuff>());
    return (1);
}

FT_TEST(test_game_skill_destroy_uninitialized_returns_invalid_state,
    "ft_skill destroy on uninitialized object is non-aborting")
{
    ft_skill skill;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, skill.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, skill.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_skill>());
    return (1);
}

FT_TEST(test_game_reputation_destroy_uninitialized_returns_invalid_state,
    "ft_reputation destroy on uninitialized object is non-aborting")
{
    ft_reputation reputation;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, reputation.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, reputation.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_reputation>());
    return (1);
}

FT_TEST(test_game_dialogue_script_destroy_uninitialized_returns_invalid_state,
    "ft_dialogue_script destroy on uninitialized object is non-aborting")
{
    ft_dialogue_script script;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, script.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, script.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_dialogue_script>());
    return (1);
}

FT_TEST(test_game_progress_tracker_destroy_uninitialized_returns_invalid_state,
    "ft_progress_tracker destroy on uninitialized object is non-aborting")
{
    ft_progress_tracker tracker;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, tracker.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, tracker.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_progress_tracker>());
    return (1);
}

FT_TEST(test_game_inventory_destroy_uninitialized_returns_invalid_state,
    "ft_inventory destroy on uninitialized object is non-aborting")
{
    ft_inventory inventory;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, inventory.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, inventory.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_inventory>());
    return (1);
}

FT_TEST(test_game_map3d_destroy_uninitialized_returns_invalid_state,
    "ft_map3d destroy on uninitialized object is non-aborting")
{
    ft_map3d map_instance;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, map_instance.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, map_instance.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_map3d>());
    return (1);
}

FT_TEST(test_game_achievement_destroy_uninitialized_returns_invalid_state,
    "ft_achievement destroy on uninitialized object is non-aborting")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, achievement.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, achievement.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_achievement>());
    return (1);
}

FT_TEST(test_game_hooks_destroy_uninitialized_returns_invalid_state,
    "ft_game_hooks destroy on uninitialized object is non-aborting")
{
    ft_game_hooks hooks;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, hooks.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, hooks.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_game_hooks>());
    return (1);
}

FT_TEST(test_game_goal_destroy_uninitialized_returns_invalid_state,
    "ft_goal destroy on uninitialized object is non-aborting")
{
    ft_goal goal;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, goal.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, goal.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_goal>());
    return (1);
}

FT_TEST(test_game_item_definition_destroy_uninitialized_is_non_aborting,
    "ft_item_definition destroy on uninitialized object is non-aborting")
{
    ft_item_definition item_definition;

    const int destroy_error = item_definition.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, item_definition.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_item_definition>());
    return (1);
}

FT_TEST(test_game_recipe_blueprint_destroy_uninitialized_is_non_aborting,
    "ft_recipe_blueprint destroy on uninitialized object is non-aborting")
{
    ft_recipe_blueprint recipe_blueprint;

    const int destroy_error = recipe_blueprint.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, recipe_blueprint.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_recipe_blueprint>());
    return (1);
}

FT_TEST(test_game_loadout_entry_destroy_uninitialized_is_non_aborting,
    "ft_loadout_entry destroy on uninitialized object is non-aborting")
{
    ft_loadout_entry loadout_entry;

    const int destroy_error = loadout_entry.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, loadout_entry.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_loadout_entry>());
    return (1);
}

FT_TEST(test_game_loadout_blueprint_destroy_uninitialized_is_non_aborting,
    "ft_loadout_blueprint destroy on uninitialized object is non-aborting")
{
    ft_loadout_blueprint loadout_blueprint;

    const int destroy_error = loadout_blueprint.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, loadout_blueprint.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_loadout_blueprint>());
    return (1);
}

FT_TEST(test_game_data_catalog_destroy_uninitialized_is_non_aborting,
    "ft_data_catalog destroy on uninitialized object is non-aborting")
{
    ft_data_catalog data_catalog;

    const int destroy_error = data_catalog.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, data_catalog.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_data_catalog>());
    return (1);
}

FT_TEST(test_game_dialogue_table_destroy_uninitialized_is_non_aborting,
    "ft_dialogue_table destroy on uninitialized object is non-aborting")
{
    ft_dialogue_table dialogue_table;

    const int destroy_error = dialogue_table.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, dialogue_table.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_dialogue_table>());
    return (1);
}

FT_TEST(test_game_path_step_destroy_uninitialized_is_non_aborting,
    "ft_path_step destroy on uninitialized object is non-aborting")
{
    ft_path_step path_step;

    const int destroy_error = path_step.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, path_step.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_path_step>());
    return (1);
}

FT_TEST(test_game_pathfinding_destroy_uninitialized_is_non_aborting,
    "ft_pathfinding destroy on uninitialized object is non-aborting")
{
    ft_pathfinding pathfinding;

    const int destroy_error = pathfinding.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, pathfinding.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_pathfinding>());
    return (1);
}

FT_TEST(test_game_world_region_destroy_uninitialized_is_non_aborting,
    "ft_world_region destroy on uninitialized object is non-aborting")
{
    ft_world_region world_region;

    const int destroy_error = world_region.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, world_region.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_world_region>());
    return (1);
}

FT_TEST(test_game_region_definition_destroy_uninitialized_is_non_aborting,
    "ft_region_definition destroy on uninitialized object is non-aborting")
{
    ft_region_definition region_definition;

    const int destroy_error = region_definition.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, region_definition.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_region_definition>());
    return (1);
}

FT_TEST(test_game_world_registry_destroy_uninitialized_is_non_aborting,
    "ft_world_registry destroy on uninitialized object is non-aborting")
{
    ft_world_registry world_registry;

    const int destroy_error = world_registry.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, world_registry.get_error());
    FT_ASSERT_EQ(1,
        expect_no_sigabrt_on_uninitialized_destructor<ft_world_registry>());
    return (1);
}

FT_TEST(test_game_quest_destroy_uninitialized_is_non_aborting,
    "ft_quest destroy on uninitialized object is non-aborting")
{
    ft_quest quest;

    const int destroy_error = quest.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, quest.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_quest>());
    return (1);
}

FT_TEST(test_game_upgrade_destroy_uninitialized_is_non_aborting,
    "ft_upgrade destroy on uninitialized object is non-aborting")
{
    ft_upgrade upgrade;

    const int destroy_error = upgrade.destroy();
    FT_ASSERT_EQ(1, is_non_aborting_uninitialized_destroy_result(
        destroy_error));
    FT_ASSERT_EQ(destroy_error, upgrade.get_error());
    FT_ASSERT_EQ(1, expect_no_sigabrt_on_uninitialized_destructor<ft_upgrade>());
    return (1);
}
