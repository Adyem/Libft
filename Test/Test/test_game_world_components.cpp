#include "../../Game/game_world.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../Game/ft_region_definition.hpp"
#include "../../Game/ft_world_region.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_crafting.hpp"
#include "../../Game/game_dialogue_table.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_world_initializes_new_subsystems, "Game world creates quest vendor and upgrade resources")
{
    ft_world world;

    FT_ASSERT_EQ(false, world.get_quest().get() == ft_nullptr);
    FT_ASSERT_EQ(false, world.get_vendor_profile().get() == ft_nullptr);
    FT_ASSERT_EQ(false, world.get_upgrade().get() == ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_new_resources, "Game world copy retains quest vendor and upgrade values")
{
    ft_world world;
    ft_sharedptr<ft_quest> quest = world.get_quest();
    ft_sharedptr<ft_vendor_profile> vendor = world.get_vendor_profile();
    ft_sharedptr<ft_upgrade> upgrade = world.get_upgrade();

    quest->set_id(9);
    quest->set_phases(3);
    vendor->set_vendor_id(42);
    vendor->set_buy_markup(1.5);
    upgrade->set_id(11);
    upgrade->set_max_level(4);
    upgrade->set_current_level(2);

    ft_world copied(world);
    FT_ASSERT_EQ(9, copied.get_quest()->get_id());
    FT_ASSERT_EQ(3, copied.get_quest()->get_phases());
    FT_ASSERT_EQ(42, copied.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(1.5, copied.get_vendor_profile()->get_buy_markup());
    FT_ASSERT_EQ(11, copied.get_upgrade()->get_id());
    FT_ASSERT_EQ(2, copied.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(4, copied.get_upgrade()->get_max_level());
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_new_resources, "Game world move transfers quest vendor and upgrade ownership")
{
    ft_world source;
    ft_sharedptr<ft_quest> quest = source.get_quest();
    ft_sharedptr<ft_vendor_profile> vendor = source.get_vendor_profile();
    ft_sharedptr<ft_upgrade> upgrade = source.get_upgrade();

    quest->set_id(13);
    vendor->set_vendor_id(77);
    vendor->set_tax_rate(0.25);
    upgrade->set_id(21);
    upgrade->set_max_level(5);
    upgrade->set_current_level(3);

    ft_world moved(ft_move(source));
    FT_ASSERT_EQ(13, moved.get_quest()->get_id());
    FT_ASSERT_EQ(77, moved.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(0.25, moved.get_vendor_profile()->get_tax_rate());
    FT_ASSERT_EQ(21, moved.get_upgrade()->get_id());
    FT_ASSERT_EQ(3, moved.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(5, moved.get_upgrade()->get_max_level());
    source.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_get_quest_propagates_errors, "Game world getters propagate quest validation failures")
{
    ft_world world;
    ft_sharedptr<ft_quest> quest = world.get_quest();

    ft_errno = ER_SUCCESS;
    quest->set_phases(-5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest->get_error());
    world.get_quest();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}


FT_TEST(test_game_world_get_vendor_profile_propagates_null_pointer_error, "Game world vendor profile getter reports missing pointer")
{
    ft_world world;
    ft_sharedptr<ft_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile.reset();
    ft_errno = ER_SUCCESS;
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    FT_ASSERT_EQ(true, vendor_profile.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_vendor_profile_propagates_errors, "Game world getters propagate vendor profile validation failures")
{
    ft_world world;
    ft_sharedptr<ft_vendor_profile> vendor_profile = world.get_vendor_profile();

    ft_errno = ER_SUCCESS;
    vendor_profile->set_vendor_id(-9);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, vendor_profile->get_error());
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}


FT_TEST(test_game_world_vendor_profile_recovers_after_replacement, "Game world vendor profile getter clears error after restoring pointer")
{
    ft_world world;
    ft_sharedptr<ft_vendor_profile> &vendor_profile = world.get_vendor_profile();

    vendor_profile.reset();
    ft_errno = ER_SUCCESS;
    world.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    vendor_profile = ft_sharedptr<ft_vendor_profile>(new ft_vendor_profile());
    vendor_profile->set_vendor_id(17);
    world.get_vendor_profile();
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(17, vendor_profile->get_vendor_id());
    return (1);
}


FT_TEST(test_game_world_get_upgrade_recovers_after_replacement, "Game world upgrade getter clears error after restoring pointer")
{
    ft_world world;
    ft_sharedptr<ft_upgrade> &upgrade = world.get_upgrade();

    upgrade.reset();
    ft_errno = ER_SUCCESS;
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    upgrade = ft_sharedptr<ft_upgrade>(new ft_upgrade());
    world.get_upgrade();
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(false, upgrade.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_upgrade_propagates_errors, "Game world getters propagate upgrade validation failures")
{
    ft_world world;
    ft_sharedptr<ft_upgrade> upgrade = world.get_upgrade();

    ft_errno = ER_SUCCESS;
    upgrade->set_id(-3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, upgrade->get_error());
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}


FT_TEST(test_game_world_copy_preserves_quest_text_fields, "Game world copy retains quest descriptive fields")
{
    ft_world world;

    world.get_quest()->set_description(ft_string("Enter the catacombs"));
    world.get_quest()->set_objective(ft_string("Light the braziers"));
    world.get_quest()->set_reward_experience(250);

    ft_world copy(world);
    FT_ASSERT_EQ(ft_string("Enter the catacombs"), copy.get_quest()->get_description());
    FT_ASSERT_EQ(ft_string("Light the braziers"), copy.get_quest()->get_objective());
    FT_ASSERT_EQ(250, copy.get_quest()->get_reward_experience());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_move_preserves_vendor_pricing_fields, "Game world move keeps vendor pricing and taxes")
{
    ft_world source;

    source.get_vendor_profile()->set_vendor_id(88);
    source.get_vendor_profile()->set_buy_markup(1.4);
    source.get_vendor_profile()->set_sell_multiplier(0.65);
    source.get_vendor_profile()->set_tax_rate(0.12);

    ft_world moved(ft_move(source));
    FT_ASSERT_EQ(88, moved.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(1.4, moved.get_vendor_profile()->get_buy_markup());
    FT_ASSERT_EQ(0.65, moved.get_vendor_profile()->get_sell_multiplier());
    FT_ASSERT_EQ(0.12, moved.get_vendor_profile()->get_tax_rate());
    source.get_vendor_profile();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_assignment_preserves_new_resources, "Game world copy assignment copies quest vendor and upgrade data")
{
    ft_world source;
    ft_world destination;

    source.get_quest()->set_id(24);
    source.get_quest()->set_phases(4);
    source.get_vendor_profile()->set_vendor_id(64);
    source.get_vendor_profile()->set_buy_markup(1.8);
    source.get_upgrade()->set_id(33);
    source.get_upgrade()->set_current_level(2);

    destination.get_quest()->set_id(1);
    destination.get_vendor_profile()->set_vendor_id(2);
    destination.get_upgrade()->set_id(3);

    destination = source;
    FT_ASSERT_EQ(24, destination.get_quest()->get_id());
    FT_ASSERT_EQ(4, destination.get_quest()->get_phases());
    FT_ASSERT_EQ(64, destination.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(1.8, destination.get_vendor_profile()->get_buy_markup());
    FT_ASSERT_EQ(33, destination.get_upgrade()->get_id());
    FT_ASSERT_EQ(2, destination.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_world_region_content, "Game world copy retains world region identifiers")
{
    ft_world world;
    ft_vector<int> region_ids;

    region_ids.push_back(5);
    region_ids.push_back(9);
    world.get_world_region()->set_world_id(3);
    world.get_world_region()->set_region_ids(region_ids);

    ft_world copy(world);
    FT_ASSERT_EQ(3, copy.get_world_region()->get_world_id());
    FT_ASSERT_EQ((size_t)2, copy.get_world_region()->get_region_ids().size());
    FT_ASSERT_EQ(5, copy.get_world_region()->get_region_ids()[0]);
    FT_ASSERT_EQ(9, copy.get_world_region()->get_region_ids()[1]);
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_preserves_registered_regions, "Game world copy maintains registry entries")
{
    ft_world world;
    ft_region_definition region_definition;
    ft_world_region world_entry;
    ft_region_definition fetched_region;
    ft_world_region fetched_world;
    ft_vector<int> region_ids;

    region_definition = ft_region_definition(21, ft_string("Shrine"), ft_string("Ancient"), 10);
    region_ids.push_back(21);
    world_entry = ft_world_region(4, region_ids);
    FT_ASSERT_EQ(ER_SUCCESS, world.get_world_registry()->register_region(region_definition));
    FT_ASSERT_EQ(ER_SUCCESS, world.get_world_registry()->register_world(world_entry));

    ft_world copied(world);
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_world_registry()->fetch_region(21, fetched_region));
    FT_ASSERT_EQ(ft_string("Shrine"), fetched_region.get_name());
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_world_registry()->fetch_world(4, fetched_world));
    FT_ASSERT_EQ((size_t)1, fetched_world.get_region_ids().size());
    FT_ASSERT_EQ(21, fetched_world.get_region_ids()[0]);
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_move_assignment_transfers_new_resources, "Game world move assignment moves quest vendor and upgrade ownership")
{
    ft_world source;
    ft_world destination;

    source.get_quest()->set_id(45);
    source.get_vendor_profile()->set_vendor_id(91);
    source.get_vendor_profile()->set_sell_multiplier(0.7);
    source.get_upgrade()->set_id(52);
    source.get_upgrade()->set_max_level(6);
    source.get_upgrade()->set_current_level(4);

    destination.get_quest()->set_id(2);
    destination.get_vendor_profile()->set_vendor_id(3);
    destination.get_upgrade()->set_id(4);

    destination = ft_move(source);
    FT_ASSERT_EQ(45, destination.get_quest()->get_id());
    FT_ASSERT_EQ(91, destination.get_vendor_profile()->get_vendor_id());
    FT_ASSERT_EQ(0.7, destination.get_vendor_profile()->get_sell_multiplier());
    FT_ASSERT_EQ(52, destination.get_upgrade()->get_id());
    FT_ASSERT_EQ(4, destination.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(6, destination.get_upgrade()->get_max_level());
    source.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_world_region_data, "Game world move transfers world region and resets source")
{
    ft_world source;
    ft_sharedptr<ft_world_region> world_region = source.get_world_region();
    ft_vector<int> region_ids;

    region_ids.push_back(12);
    region_ids.push_back(18);
    world_region->set_world_id(8);
    world_region->set_region_ids(region_ids);

    ft_world moved(ft_move(source));
    FT_ASSERT_EQ(8, moved.get_world_region()->get_world_id());
    FT_ASSERT_EQ((size_t)2, moved.get_world_region()->get_region_ids().size());
    FT_ASSERT_EQ(12, moved.get_world_region()->get_region_ids()[0]);
    FT_ASSERT_EQ(18, moved.get_world_region()->get_region_ids()[1]);
    source.get_world_region();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}


FT_TEST(test_game_world_reuses_shared_new_components, "Game world getters return shared quest vendor and upgrade pointers")
{
    ft_world world;
    ft_sharedptr<ft_quest> first_quest = world.get_quest();
    ft_sharedptr<ft_vendor_profile> first_vendor = world.get_vendor_profile();
    ft_sharedptr<ft_upgrade> first_upgrade = world.get_upgrade();

    first_quest->set_id(77);
    first_vendor->set_tax_rate(0.15);
    first_upgrade->set_current_level(3);
    FT_ASSERT_EQ(77, world.get_quest()->get_id());
    FT_ASSERT_EQ(0.15, world.get_vendor_profile()->get_tax_rate());
    FT_ASSERT_EQ(3, world.get_upgrade()->get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_get_world_region_propagates_null_pointer_error, "Game world reports missing world region pointer")
{
    ft_world world;
    ft_sharedptr<ft_world_region> &world_region = world.get_world_region();

    world_region.reset();
    ft_errno = ER_SUCCESS;
    world.get_world_region();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    FT_ASSERT_EQ(true, world_region.get() == ft_nullptr);
    return (1);
}


FT_TEST(test_game_world_get_world_region_recovers_after_replacement, "Game world clears error once world region is restored")
{
    ft_world world;
    ft_sharedptr<ft_world_region> &world_region = world.get_world_region();

    world_region.reset();
    ft_errno = ER_SUCCESS;
    world.get_world_region();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, ft_errno);
    world_region = ft_sharedptr<ft_world_region>(new ft_world_region());
    world_region->set_world_id(7);
    world.get_world_region();
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(7, world_region->get_world_id());
    return (1);
}


FT_TEST(test_game_world_exposes_economy_crafting_and_dialogue, "Game world provides economy, crafting, and dialogue accessors")
{
    ft_world world;

    FT_ASSERT_NE((void *)ft_nullptr, world.get_economy_table().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_crafting().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_dialogue_table().get());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table().get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_crafting().get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_dialogue_table().get_error());
    return (1);
}


FT_TEST(test_game_world_registers_economy_entries_through_shared_tables, "Game world registers vendor and currency data through economy table")
{
    ft_world world;
    ft_price_definition potion_price;
    ft_vendor_profile vendor_profile;
    ft_currency_rate gold_rate;

    potion_price.set_item_id(42);
    potion_price.set_rarity(3);
    potion_price.set_base_value(150);
    potion_price.set_minimum_value(75);
    potion_price.set_maximum_value(300);
    vendor_profile.set_vendor_id(12);
    vendor_profile.set_buy_markup(1.2);
    vendor_profile.set_sell_multiplier(0.6);
    vendor_profile.set_tax_rate(0.08);
    gold_rate.set_currency_id(1);
    gold_rate.set_rate_to_base(1.0);
    gold_rate.set_display_precision(2);

    world.get_economy_table()->register_price_definition(potion_price);
    world.get_economy_table()->register_vendor_profile(vendor_profile);
    world.get_economy_table()->register_currency_rate(gold_rate);
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table()->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table().get_error());
    return (1);
}


FT_TEST(test_game_world_propagates_null_shared_components, "Game world reports errors when shared components are missing")
{
    ft_world world;

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


FT_TEST(test_game_world_get_upgrade_propagates_level_error, "Game world upgrade getter propagates invalid level errors")
{
    ft_world world;
    ft_sharedptr<ft_upgrade> upgrade = world.get_upgrade();

    upgrade->set_max_level(2);
    upgrade->set_current_level(5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, upgrade->get_error());
    ft_errno = ER_SUCCESS;
    world.get_upgrade();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}


FT_TEST(test_game_world_upgrade_error_clears_after_valid_level, "Game world clears upgrade errors once level is valid")
{
    ft_world world;
    ft_sharedptr<ft_upgrade> upgrade = world.get_upgrade();

    upgrade->set_max_level(1);
    upgrade->set_current_level(4);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, upgrade->get_error());

    upgrade->set_max_level(5);
    upgrade->set_current_level(3);
    world.get_upgrade();
    FT_ASSERT_EQ(3, upgrade->get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


FT_TEST(test_game_world_vendor_profile_error_clears_after_valid_id, "Game world clears vendor profile errors after valid id reset")
{
    ft_world world;
    ft_sharedptr<ft_vendor_profile> vendor_profile = world.get_vendor_profile();

    vendor_profile->set_vendor_id(-3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, vendor_profile->get_error());

    vendor_profile->set_vendor_id(88);
    world.get_vendor_profile();
    FT_ASSERT_EQ(88, vendor_profile->get_vendor_id());
    FT_ASSERT_EQ(ER_SUCCESS, vendor_profile->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    return (1);
}


