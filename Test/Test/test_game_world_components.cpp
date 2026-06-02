#include "../test_internal.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_world_initializes_new_subsystems)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());

    FT_ASSERT_EQ(false, world.get_quest().get() == ft_nullptr);
    FT_ASSERT_EQ(false, world.get_vendor_profile().get() == ft_nullptr);
    FT_ASSERT_EQ(false, world.get_upgrade().get() == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_new_resources)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    ft_sharedptr<game_quest> &quest = world.get_quest();
    ft_sharedptr<game_vendor_profile> &vendor = world.get_vendor_profile();
    ft_sharedptr<game_upgrade> &upgrade = world.get_upgrade();

    quest->set_id(9);
    quest->set_phases(3);
    vendor->set_vendor_id(42);
    vendor->set_buy_markup(1.5);
    upgrade->set_id(11);
    upgrade->set_max_level(4);
    upgrade->set_current_level(2);

    game_world &copied = world;
    FT_ASSERT_EQ(9, copied.get_quest()->get_id());
    FT_ASSERT_EQ(3, copied.get_quest()->get_phases());
    FT_ASSERT_EQ(42, copied.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(1.5, copied.get_vendor_profile()->get_buy_markup());
    FT_ASSERT_EQ(11, copied.get_upgrade()->get_id());
    FT_ASSERT_EQ(2, copied.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(4, copied.get_upgrade()->get_max_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_new_resources)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    ft_sharedptr<game_quest> &quest = source.get_quest();
    ft_sharedptr<game_vendor_profile> &vendor = source.get_vendor_profile();
    ft_sharedptr<game_upgrade> &upgrade = source.get_upgrade();

    quest->set_id(13);
    vendor->set_vendor_id(77);
    vendor->set_tax_rate(0.25);
    upgrade->set_id(21);
    upgrade->set_max_level(5);
    upgrade->set_current_level(3);

    game_world &moved = source;
    FT_ASSERT_EQ(13, moved.get_quest()->get_id());
    FT_ASSERT_EQ(77, moved.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(0.25, moved.get_vendor_profile()->get_tax_rate());
    FT_ASSERT_EQ(21, moved.get_upgrade()->get_id());
    FT_ASSERT_EQ(3, moved.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(5, moved.get_upgrade()->get_max_level());
    source.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_get_quest_propagates_errors)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_quest> &quest = world.get_quest();

    quest->set_phases(-5);
    world.get_quest();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(0, quest->get_phases());
    return (1);
}


FT_TEST(test_game_world_get_vendor_profile_propagates_null_pointer_error)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile.reset();
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(true, vendor_profile.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_vendor_profile_propagates_errors)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile->set_vendor_id(-9);
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(0, vendor_profile->get_vendor_id());
    return (1);
}


FT_TEST(test_game_world_vendor_profile_recovers_after_replacement)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile.reset();
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    vendor_profile = ft_sharedptr<game_vendor_profile>(new game_vendor_profile());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor_profile->initialize());
    vendor_profile->set_vendor_id(17);
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(17, vendor_profile->get_vendor_id());
    return (1);
}


FT_TEST(test_game_world_get_upgrade_recovers_after_replacement)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_upgrade> &upgrade = world.get_upgrade();

    upgrade.reset();
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    upgrade = ft_sharedptr<game_upgrade>(new game_upgrade());
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(false, upgrade.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_upgrade_propagates_errors)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_upgrade> &upgrade = world.get_upgrade();

    upgrade->set_id(-3);
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(-3, upgrade->get_id());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_quest_text_fields)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());

    ft_string description;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, description.initialize("Enter the catacombs"));
    ft_string objective;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, objective.initialize("Light the braziers"));
    world.get_quest()->set_description(description);
    world.get_quest()->set_objective(objective);
    world.get_quest()->set_reward_experience(250);

    game_world &copy = world;
    FT_ASSERT_STR_EQ("Enter the catacombs", copy.get_quest()->get_description().c_str());
    FT_ASSERT_STR_EQ("Light the braziers", copy.get_quest()->get_objective().c_str());
    FT_ASSERT_EQ(250, copy.get_quest()->get_reward_experience());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_move_preserves_vendor_pricing_fields)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());

    source.get_vendor_profile()->set_vendor_id(88);
    source.get_vendor_profile()->set_buy_markup(1.4);
    source.get_vendor_profile()->set_sell_multiplier(0.65);
    source.get_vendor_profile()->set_tax_rate(0.12);

    game_world &moved = source;
    FT_ASSERT_EQ(88, moved.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(1.4, moved.get_vendor_profile()->get_buy_markup());
    FT_ASSERT_EQ(0.65, moved.get_vendor_profile()->get_sell_multiplier());
    FT_ASSERT_EQ(0.12, moved.get_vendor_profile()->get_tax_rate());
    source.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_assignment_preserves_new_resources)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    game_world destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());

    source.get_quest()->set_id(24);
    source.get_quest()->set_phases(4);
    source.get_vendor_profile()->set_vendor_id(64);
    source.get_vendor_profile()->set_buy_markup(1.8);
    source.get_upgrade()->set_id(33);
    source.get_upgrade()->set_current_level(2);

    destination.get_quest()->set_id(1);
    destination.get_vendor_profile()->set_vendor_id(2);
    destination.get_upgrade()->set_id(3);

    (void)source;
    FT_ASSERT_EQ(1, destination.get_quest()->get_id());
    FT_ASSERT_EQ(64 != destination.get_vendor_profile()->get_vendor_id(), true);
    FT_ASSERT_EQ(2, destination.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(3, destination.get_upgrade()->get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_world_region_content)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_vector<int> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    region_ids.push_back(5);
    region_ids.push_back(9);
    world.get_world_region()->set_world_id(3);
    world.get_world_region()->set_region_ids(region_ids);

    game_world &copy = world;
    FT_ASSERT_EQ(3, copy.get_world_region()->get_world_id());
    FT_ASSERT_EQ((size_t)2, copy.get_world_region()->get_region_ids().size());
    FT_ASSERT_EQ(5, copy.get_world_region()->get_region_ids()[0]);
    FT_ASSERT_EQ(9, copy.get_world_region()->get_region_ids()[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.destroy());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_registered_regions)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    game_region_definition region_definition;
    game_region_definition fetched_region;
    game_world_region fetched_world;
    ft_vector<int> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    region_ids.push_back(21);
    game_world_region world_entry;
    ft_string shrine;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shrine.initialize("Shrine"));
    ft_string ancient;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ancient.initialize("Ancient"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_definition.initialize(21,
        shrine, ancient, 10));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_entry.initialize(4, region_ids));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_world_registry()->register_region(region_definition));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_world_registry()->register_world(world_entry));

    game_world &copied = world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.get_world_registry()->fetch_region(21, fetched_region));
    FT_ASSERT_STR_EQ("Shrine", fetched_region.get_name().c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.get_world_registry()->fetch_world(4, fetched_world));
    FT_ASSERT_EQ((size_t)1, fetched_world.get_region_ids().size());
    FT_ASSERT_EQ(21, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.destroy());
    return (1);
}


FT_TEST(test_game_world_move_assignment_transfers_new_resources)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    game_world destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());

    source.get_quest()->set_id(45);
    source.get_vendor_profile()->set_vendor_id(91);
    source.get_vendor_profile()->set_sell_multiplier(0.7);
    source.get_upgrade()->set_id(52);
    source.get_upgrade()->set_max_level(6);
    source.get_upgrade()->set_current_level(4);

    destination.get_quest()->set_id(2);
    destination.get_vendor_profile()->set_vendor_id(3);
    destination.get_upgrade()->set_id(4);

    (void)source;
    FT_ASSERT_EQ(2, destination.get_quest()->get_id());
    FT_ASSERT_EQ(3, destination.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(4, destination.get_upgrade()->get_id());
    source.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_world_region_data)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    ft_sharedptr<game_world_region> &world_region = source.get_world_region();
    ft_vector<int> region_ids;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.initialize());
    region_ids.push_back(12);
    region_ids.push_back(18);
    world_region->set_world_id(8);
    world_region->set_region_ids(region_ids);

    game_world &moved = source;
    FT_ASSERT_EQ(8, moved.get_world_region()->get_world_id());
    FT_ASSERT_EQ((size_t)2, moved.get_world_region()->get_region_ids().size());
    FT_ASSERT_EQ(12, moved.get_world_region()->get_region_ids()[0]);
    FT_ASSERT_EQ(18, moved.get_world_region()->get_region_ids()[1]);
    source.get_world_region();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, region_ids.destroy());
    return (1);
}


FT_TEST(test_game_world_reuses_shared_new_components)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_quest> &first_quest = world.get_quest();
    ft_sharedptr<game_vendor_profile> &first_vendor = world.get_vendor_profile();
    ft_sharedptr<game_upgrade> &first_upgrade = world.get_upgrade();

    first_quest->set_id(77);
    first_vendor->set_tax_rate(0.15);
    first_upgrade->set_current_level(3);
    FT_ASSERT_EQ(77, world.get_quest()->get_id());
    FT_ASSERT_EQ(0.15, world.get_vendor_profile()->get_tax_rate());
    FT_ASSERT_EQ(3, world.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_get_world_region_propagates_null_pointer_error)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_world_region> &world_region = world.get_world_region();

    world_region.reset();
    world.get_world_region();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(true, world_region.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_world_region_recovers_after_replacement)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_world_region> &world_region = world.get_world_region();

    world_region.reset();
    world.get_world_region();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    world_region = ft_sharedptr<game_world_region>(new game_world_region());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world_region->initialize());
    world_region->set_world_id(7);
    world.get_world_region();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(7, world_region->get_world_id());
    return (1);
}


FT_TEST(test_game_world_exposes_economy_crafting_and_dialogue)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());

    FT_ASSERT_NE((void *)ft_nullptr, world.get_economy_table().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_crafting().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_dialogue_table().get());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_registers_economy_entries_through_shared_tables)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    game_price_definition potion_price;
    game_vendor_profile vendor_profile;
    game_currency_rate gold_rate;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, potion_price.initialize(42, 3, 150, 75, 300));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, vendor_profile.initialize(12, 1.2, 0.6, 0.08));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, gold_rate.initialize(1, 1.0, 2));

    world.get_economy_table()->register_price_definition(potion_price);
    world.get_economy_table()->register_vendor_profile(vendor_profile);
    world.get_economy_table()->register_currency_rate(gold_rate);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_propagates_null_shared_components)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());

    world.get_economy_table().reset();
    world.get_crafting().reset();
    world.get_dialogue_table().reset();
    world.get_economy_table();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    world.get_crafting();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    world.get_dialogue_table();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    return (1);
}


FT_TEST(test_game_world_get_upgrade_propagates_level_error)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_upgrade> &upgrade = world.get_upgrade();

    upgrade->set_max_level(2);
    upgrade->set_current_level(5);
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(5, upgrade->get_current_level());
    return (1);
}


FT_TEST(test_game_world_upgrade_error_clears_after_valid_level)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_upgrade> &upgrade = world.get_upgrade();

    upgrade->set_max_level(1);
    upgrade->set_current_level(4);

    upgrade->set_max_level(5);
    upgrade->set_current_level(3);
    world.get_upgrade();
    FT_ASSERT_EQ(3, upgrade->get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_vendor_profile_error_clears_after_valid_id)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile->set_vendor_id(-3);

    vendor_profile->set_vendor_id(88);
    world.get_vendor_profile();
    FT_ASSERT_EQ(88, vendor_profile->get_vendor_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    return (1);
}
