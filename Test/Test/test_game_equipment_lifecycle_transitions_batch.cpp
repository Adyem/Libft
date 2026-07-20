#include "../../Modules/Game/game_equipment.hpp"
#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void equipment_move_from_uninitialised(game_equipment &value)
{
    game_equipment source;

    (void)value.move(source);
    return ;
}

static void equipment_destroy_item_handle(ft_sharedptr<game_item> *item_pointer)
{
    if (item_pointer == ft_nullptr)
        return ;
    (void)item_pointer->destroy();
    delete item_pointer;
    return ;
}

static ft_bool equipment_slot_contains_item(game_equipment &value, int32_t slot)
{
    ft_sharedptr<game_item> *item_pointer;
    ft_bool contains_item;

    item_pointer = value.get_item(slot);
    if (item_pointer == ft_nullptr)
        return (FT_FALSE);
    contains_item = item_pointer->get() != ft_nullptr;
    equipment_destroy_item_handle(item_pointer);
    return (contains_item);
}

FT_TEST(test_game_equipment_move_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
                        equipment_move_from_uninitialised));
    return (1);
}

FT_TEST(test_game_equipment_move_from_destroyed_source_destroys_destination)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_replaces_initialized_destination)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> item(new game_item());
    ft_sharedptr<game_item> *found_item;

    found_item = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_HEAD, item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    found_item = destination.get_item(EQUIP_HEAD);
    FT_ASSERT(found_item != ft_nullptr);
    FT_ASSERT(found_item->get() != ft_nullptr);
    equipment_destroy_item_handle(found_item);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_self_move_preserves_empty_slots)
{
    game_equipment value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(FT_FALSE, equipment_slot_contains_item(value, EQUIP_HEAD));
    return (1);
}

FT_TEST(test_game_equipment_move_source_can_be_reinitialized)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_FALSE, equipment_slot_contains_item(source, EQUIP_CHEST));
    return (1);
}

FT_TEST(test_game_equipment_destroyed_destination_can_be_reinitialized)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_FALSE,
                 equipment_slot_contains_item(destination, EQUIP_WEAPON));
    return (1);
}

FT_TEST(test_game_equipment_move_transfers_head_slot)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> item(new game_item());
    ft_sharedptr<game_item> *found_item;

    found_item = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_HEAD, item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    found_item = destination.get_item(EQUIP_HEAD);
    FT_ASSERT(found_item != ft_nullptr);
    FT_ASSERT(found_item->get() != ft_nullptr);
    equipment_destroy_item_handle(found_item);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_transfers_chest_slot)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> item(new game_item());
    ft_sharedptr<game_item> *found_item;

    found_item = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_CHEST, item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    found_item = destination.get_item(EQUIP_CHEST);
    FT_ASSERT(found_item != ft_nullptr);
    FT_ASSERT(found_item->get() != ft_nullptr);
    equipment_destroy_item_handle(found_item);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_transfers_weapon_slot)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> item(new game_item());
    ft_sharedptr<game_item> *found_item;

    found_item = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_WEAPON, item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    found_item = destination.get_item(EQUIP_WEAPON);
    FT_ASSERT(found_item != ft_nullptr);
    FT_ASSERT(found_item->get() != ft_nullptr);
    equipment_destroy_item_handle(found_item);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_transfers_all_three_slots)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> head_item(new game_item());
    ft_sharedptr<game_item> chest_item(new game_item());
    ft_sharedptr<game_item> weapon_item(new game_item());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, head_item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chest_item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, weapon_item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_HEAD, head_item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_CHEST, chest_item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_WEAPON, weapon_item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_TRUE,
                 equipment_slot_contains_item(destination, EQUIP_HEAD));
    FT_ASSERT_EQ(FT_TRUE,
                 equipment_slot_contains_item(destination, EQUIP_CHEST));
    FT_ASSERT_EQ(FT_TRUE,
                 equipment_slot_contains_item(destination, EQUIP_WEAPON));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, head_item.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, chest_item.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, weapon_item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_leaves_source_empty_after_reinitialization)
{
    game_equipment source;
    game_equipment destination;
    ft_sharedptr<game_item> item(new game_item());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.equip(EQUIP_HEAD, item));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_FALSE, equipment_slot_contains_item(source, EQUIP_HEAD));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, item.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_preserves_success_error_state)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_game_equipment_move_with_thread_safe_source_is_successful)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_FALSE, destination.is_thread_safe());
    return (1);
}

FT_TEST(test_game_equipment_move_cleans_thread_safe_destination)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_FALSE, destination.is_thread_safe());
    return (1);
}

FT_TEST(test_game_equipment_repeated_move_chain_preserves_initialization)
{
    game_equipment first;
    game_equipment second;
    game_equipment third;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.move(first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, third.move(second));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, third.get_error());
    return (1);
}

FT_TEST(test_game_equipment_move_then_destroy_destination_is_safe)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_then_destroy_source_is_safe)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    return (1);
}

FT_TEST(test_game_equipment_move_empty_initialized_source_is_successful)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_FALSE,
                 equipment_slot_contains_item(destination, EQUIP_HEAD));
    FT_ASSERT_EQ(FT_FALSE,
                 equipment_slot_contains_item(destination, EQUIP_CHEST));
    FT_ASSERT_EQ(FT_FALSE,
                 equipment_slot_contains_item(destination, EQUIP_WEAPON));
    return (1);
}

FT_TEST(test_game_equipment_move_destination_can_restart_thread_safety)
{
    game_equipment source;
    game_equipment destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, destination.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_equipment_move_destination_destructor_after_cleanup_is_safe)
{
    game_equipment source;
    game_equipment *destination;

    destination = new game_equipment();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination->move(source));
    delete destination;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
