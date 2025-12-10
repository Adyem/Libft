#include "../../Game/game_character.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/game_map3d.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_world_registry.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_event_scheduler.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/map.hpp"
#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../JSon/json.hpp"
#include <climits>
#include <cstdio>

int test_game_simulation(void)
{
    ft_character hero;
    hero.set_hit_points(50);
    hero.set_might(10);
    hero.set_physical_armor(5);

    ft_map3d grid(3, 3, 1, 0);
    grid.set(1, 1, 0, 1);
    hero.set_x(1);
    hero.set_y(1);
    hero.set_z(0);

    ft_buff strength;
    strength.set_id(1);
    strength.set_modifier1(5);
    hero.get_buffs().insert(strength.get_id(), strength);
    hero.set_might(hero.get_might() + strength.get_modifier1());
    if (hero.get_might() != 15)
        return (0);

    ft_debuff weakness;
    weakness.set_id(2);
    weakness.set_modifier1(-2);
    hero.get_debuffs().insert(weakness.get_id(), weakness);
    hero.set_physical_armor(hero.get_physical_armor() + weakness.get_modifier1());
    if (hero.get_physical_armor() != 3)
        return (0);

    ft_upgrade upgrade;
    upgrade.set_id(1);
    upgrade.set_modifier1(3);
    hero.get_upgrades().insert(upgrade.get_id(), upgrade);
    Pair<int, ft_upgrade>* uentry = hero.get_upgrades().find(1);
    if (uentry == hero.get_upgrades().end())
        return (0);
    hero.set_might(hero.get_might() + uentry->value.get_modifier1());
    if (hero.get_might() != 18)
        return (0);

    ft_quest quest;
    quest.set_id(1);
    quest.set_phases(2);
    hero.get_quests().insert(quest.get_id(), quest);
    Pair<int, ft_quest>* qentry = hero.get_quests().find(1);
    if (qentry == hero.get_quests().end())
        return (0);
    qentry->value.set_current_phase(1);
    if (qentry->value.get_current_phase() != 1)
        return (0);

    hero.get_reputation().set_milestone(1, 10);
    hero.get_reputation().add_current_rep(4);
    if (hero.get_reputation().get_current_rep() != 4 ||
        hero.get_reputation().get_total_rep() != 4)
        return (0);

    ft_sharedptr<ft_world> overworld(new ft_world());
    ft_sharedptr<ft_event> meeting(new ft_event());
    meeting->set_id(1);
    meeting->set_duration(5);
    overworld->schedule_event(meeting);
    overworld->update_events(overworld, 1);
    ft_vector<ft_sharedptr<ft_event> > events;
    overworld->get_event_scheduler()->dump_events(events);
    if (events.size() != 1 || events[0]->get_duration() != 4)
        return (0);

    ft_inventory pack(2);
    ft_sharedptr<ft_item> potion(new ft_item());
    potion->set_item_id(1);
    potion->set_max_stack(10);
    potion->set_stack_size(5);
    if (pack.add_item(potion) != FT_ER_SUCCESSS)
        return (0);
    ft_sharedptr<ft_item> more(new ft_item());
    more->set_item_id(1);
    more->set_max_stack(10);
    more->set_stack_size(3);
    pack.add_item(more);
    Pair<int, ft_sharedptr<ft_item> > *ientry = pack.get_items().find(0);
    if (ientry == pack.get_items().end() || ientry->value->get_stack_size() != 8)
        return (0);

    if (grid.get(hero.get_x(), hero.get_y(), hero.get_z()) != 1)
        return (0);

    return (1);
}

int test_item_basic(void)
{
    ft_item item;
    item.set_item_id(1);
    item.set_max_stack(10);
    item.set_stack_size(3);
    item.set_modifier1_id(5);
    item.set_modifier1_value(2);
    item.set_width(2);
    item.set_height(3);
    if (item.get_item_id() != 1 || item.get_max_stack() != 10 ||
        item.get_stack_size() != 3 || item.get_modifier1_id() != 5 ||
        item.get_modifier1_value() != 2 || item.get_width() != 2 ||
        item.get_height() != 3)
        return (0);
    return (1);
}

int test_character_valor(void)
{
    ft_character hero;
    hero.set_valor(42);
    return (hero.get_valor() == 42);
}

int test_character_add_sub_coins(void)
{
    ft_character hero;
    hero.add_coins(10);
    hero.sub_coins(3);
    return (hero.get_coins() == 7);
}

int test_character_add_sub_valor(void)
{
    ft_character hero;
    hero.add_valor(5);
    hero.sub_valor(2);
    return (hero.get_valor() == 3);
}

int test_game_save_load(void)
{
    ft_character hero;
    hero.set_hit_points(42);
    ft_world world;
    ft_sharedptr<ft_event> event(new ft_event());
    event->set_id(1);
    event->set_duration(5);
    world.schedule_event(event);
    ft_inventory inventory;
    if (world.save_to_file("test_save.json", hero, inventory) != FT_ER_SUCCESSS)
        return (0);
    ft_character loaded_hero;
    ft_world loaded_world;
    ft_inventory loaded_inventory;
    if (loaded_world.load_from_file("test_save.json", loaded_hero, loaded_inventory) != FT_ER_SUCCESSS)
        return (0);
    ft_vector<ft_sharedptr<ft_event> > loaded_events;
    loaded_world.get_event_scheduler()->dump_events(loaded_events);
    remove("test_save.json");
    if (loaded_events.size() != 1)
        return (0);
    if (loaded_events[0]->get_duration() != 5)
        return (0);
    if (loaded_hero.get_hit_points() != 42)
        return (0);
    return (1);
}

int test_buff_subtracters(void)
{
    ft_buff buff;
    buff.set_duration(10);
    buff.sub_duration(3);
    buff.set_modifier1(5);
    buff.sub_modifier1(2);
    buff.set_modifier2(4);
    buff.sub_modifier2(1);
    buff.set_modifier3(6);
    buff.sub_modifier3(6);
    buff.set_modifier4(8);
    buff.sub_modifier4(3);
    return (buff.get_duration() == 7 && buff.get_modifier1() == 3 &&
            buff.get_modifier2() == 3 && buff.get_modifier3() == 0 &&
            buff.get_modifier4() == 5);
}

int test_debuff_subtracters(void)
{
    ft_debuff debuff;
    debuff.set_duration(8);
    debuff.sub_duration(2);
    debuff.set_modifier1(-1);
    debuff.sub_modifier1(-1);
    debuff.set_modifier2(3);
    debuff.sub_modifier2(1);
    debuff.set_modifier3(2);
    debuff.sub_modifier3(2);
    debuff.set_modifier4(0);
    debuff.sub_modifier4(0);
    return (debuff.get_duration() == 6 && debuff.get_modifier1() == 0 &&
            debuff.get_modifier2() == 2 && debuff.get_modifier3() == 0 &&
            debuff.get_modifier4() == 0);
}

int test_event_subtracters(void)
{
    ft_event ev;
    ev.set_duration(5);
    ev.sub_duration(1);
    ev.set_modifier1(4);
    ev.sub_modifier1(2);
    ev.set_modifier2(3);
    ev.sub_modifier2(3);
    ev.set_modifier3(0);
    ev.sub_modifier3(0);
    ev.set_modifier4(-2);
    ev.sub_modifier4(-1);
    return (ev.get_duration() == 4 && ev.get_modifier1() == 2 &&
            ev.get_modifier2() == 0 && ev.get_modifier3() == 0 &&
            ev.get_modifier4() == -1);
}

FT_TEST(test_game_event_sub_duration_prevents_underflow, "ft_event::sub_duration rejects underflow")
{
    ft_event event;

    event.set_duration(3);
    ft_errno = FT_ER_SUCCESSS;
    event.sub_duration(5);
    FT_ASSERT_EQ(3, event.get_duration());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, event.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    event.set_duration(3);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    event.sub_duration(3);
    FT_ASSERT_EQ(0, event.get_duration());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, event.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_event_add_duration_detects_overflow, "ft_event::add_duration rejects overflow")
{
    ft_event event;

    event.set_duration(INT_MAX - 2);
    ft_errno = FT_ER_SUCCESSS;
    event.add_duration(5);
    FT_ASSERT_EQ(INT_MAX - 2, event.get_duration());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, event.get_error());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);

    ft_errno = FT_ERR_OUT_OF_RANGE;
    event.add_duration(2);
    FT_ASSERT_EQ(INT_MAX, event.get_duration());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, event.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_game_event_add_duration_rejects_negative, "ft_event::add_duration rejects negative input")
{
    ft_event event;

    event.set_duration(4);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, event.add_duration(-1));
    FT_ASSERT_EQ(4, event.get_duration());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, event.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_ER_SUCCESSS, event.add_duration(2));
    FT_ASSERT_EQ(6, event.get_duration());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, event.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_inventory_remove_item_releases_usage, "Game: removing items releases slots and weight")
{
    ft_inventory inventory(3, 20);
    ft_sharedptr<ft_item> stack(new ft_item());

    stack->set_item_id(7);
    stack->set_max_stack(10);
    stack->set_stack_size(3);
    stack->set_width(2);
    stack->set_height(1);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(stack));
    FT_ASSERT_EQ(2u, inventory.get_used());
    FT_ASSERT_EQ(3, inventory.get_current_weight());
    inventory.remove_item(0);
    FT_ASSERT_EQ(0u, inventory.get_used());
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(false, inventory.has_item(7));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.get_error());
    return (1);
}

FT_TEST(test_inventory_count_rarity_sums_stacks, "Game: count_rarity returns total stacked quantity")
{
    ft_inventory inventory(4, 20);
    ft_sharedptr<ft_item> first(new ft_item());
    ft_sharedptr<ft_item> second(new ft_item());

    first->set_item_id(1);
    first->set_rarity(2);
    first->set_max_stack(10);
    first->set_stack_size(2);
    second->set_item_id(2);
    second->set_rarity(2);
    second->set_max_stack(10);
    second->set_stack_size(5);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(first));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(second));
    FT_ASSERT_EQ(7, inventory.count_rarity(2));
    FT_ASSERT_EQ(true, inventory.has_rarity(2));
    FT_ASSERT_EQ(false, inventory.has_rarity(3));
    return (1);
}

FT_TEST(test_character_move_updates_coordinates, "Game: move updates character coordinates relative to current position")
{
    ft_character hero;

    hero.set_x(1);
    hero.set_y(2);
    hero.set_z(3);
    hero.move(2, -1, 1);
    FT_ASSERT_EQ(3, hero.get_x());
    FT_ASSERT_EQ(1, hero.get_y());
    FT_ASSERT_EQ(4, hero.get_z());
    return (1);
}

FT_TEST(test_character_restore_armor_recovers_all, "Game: restore_armor brings both armor pools back to base values")
{
    ft_character hero;

    hero.set_physical_armor(12);
    hero.set_magic_armor(9);
    hero.set_damage_rule(FT_DAMAGE_RULE_BUFFER);
    hero.take_damage(5, FT_DAMAGE_PHYSICAL);
    hero.take_damage(3, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(7, hero.get_current_physical_armor());
    FT_ASSERT_EQ(6, hero.get_current_magic_armor());
    hero.restore_armor();
    FT_ASSERT_EQ(12, hero.get_current_physical_armor());
    FT_ASSERT_EQ(9, hero.get_current_magic_armor());
    return (1);
}

FT_TEST(test_inventory_copy_preserves_stacks, "Game: copying inventories keeps item stacks intact")
{
    ft_inventory original(5, 15);
    ft_sharedptr<ft_item> stack(new ft_item());

    stack->set_item_id(3);
    stack->set_max_stack(10);
    stack->set_stack_size(4);
    stack->set_width(1);
    stack->set_height(1);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original.add_item(stack));
    FT_ASSERT_EQ(4, original.count_item(3));
    ft_inventory duplicate(original);
    original.remove_item(0);
    FT_ASSERT_EQ(0, original.count_item(3));
    FT_ASSERT_EQ(4, duplicate.count_item(3));
    FT_ASSERT_EQ(0, original.get_current_weight());
    FT_ASSERT_EQ(4, duplicate.get_current_weight());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, duplicate.get_error());
    return (1);
}

int test_upgrade_subtracters(void)
{
    ft_upgrade up;
    up.set_current_level(5);
    up.sub_level(2);
    up.set_modifier1(3);
    up.sub_modifier1(1);
    up.set_modifier2(4);
    up.sub_modifier2(4);
    up.set_modifier3(-2);
    up.sub_modifier3(-2);
    up.set_modifier4(7);
    up.sub_modifier4(3);
    return (up.get_current_level() == 3 && up.get_modifier1() == 2 &&
            up.get_modifier2() == 0 && up.get_modifier3() == 0 &&
            up.get_modifier4() == 4);
}

int test_item_stack_subtract(void)
{
    ft_item item;
    item.set_max_stack(10);
    item.set_stack_size(7);
    item.sub_from_stack(3);
    return (item.get_stack_size() == 4);
}

int test_character_level(void)
{
    ft_character hero;
    int levels[] = {0, 100, 300};
    hero.get_experience_table().set_levels(levels, 3);
    hero.set_experience(150);
    return (hero.get_level() == 2);
}

int test_quest_progress(void)
{
    ft_quest q;
    q.set_phases(3);
    if (q.is_complete())
        return (0);
    q.advance_phase();
    if (q.get_current_phase() != 1 || q.is_complete())
        return (0);
    q.advance_phase();
    q.advance_phase();
    if (!q.is_complete() || q.get_current_phase() != 3)
        return (0);
    q.advance_phase();
    return (q.get_current_phase() == 3);
}

int test_restore_individual_armor(void)
{
    ft_character hero;
    hero.set_physical_armor(10);
    hero.set_magic_armor(8);
    hero.set_damage_rule(FT_DAMAGE_RULE_BUFFER);
    hero.take_damage(5, FT_DAMAGE_PHYSICAL);
    hero.take_damage(3, FT_DAMAGE_MAGICAL);
    if (hero.get_current_physical_armor() != 5 || hero.get_current_magic_armor() != 5)
        return (0);
    hero.restore_physical_armor();
    if (hero.get_current_physical_armor() != 10 || hero.get_current_magic_armor() != 5)
        return (0);
    hero.restore_magic_armor();
    if (hero.get_current_magic_armor() != 8)
        return (0);
    return (1);
}

int test_character_serialization_damage(void)
{
    ft_character hero;
    hero.set_hit_points(50);
    hero.set_physical_armor(10);
    hero.set_magic_armor(8);
    hero.set_damage_rule(FT_DAMAGE_RULE_BUFFER);
    hero.take_damage(4, FT_DAMAGE_PHYSICAL);
    hero.take_damage(2, FT_DAMAGE_MAGICAL);
    json_group *group = serialize_character(hero);
    if (!group)
        return (0);
    ft_character clone;
    deserialize_character(clone, group);
    json_free_groups(group);
    if (clone.get_current_physical_armor() != 6)
        return (0);
    if (clone.get_current_magic_armor() != 6)
        return (0);
    if (clone.get_damage_rule() != FT_DAMAGE_RULE_BUFFER)
        return (0);
    return (1);
}

FT_TEST(test_world_region_setters_replace_ids, "Game: set_region_ids overwrites stored identifiers")
{
    ft_world_region world_region;
    ft_vector<int> first_ids;
    ft_vector<int> second_ids;

    first_ids.push_back(3);
    first_ids.push_back(7);
    first_ids.push_back(9);
    world_region.set_world_id(12);
    world_region.set_region_ids(first_ids);
    FT_ASSERT_EQ(12, world_region.get_world_id());
    FT_ASSERT_EQ(3u, world_region.get_region_ids().size());
    FT_ASSERT_EQ(3, world_region.get_region_ids()[0]);
    FT_ASSERT_EQ(7, world_region.get_region_ids()[1]);
    FT_ASSERT_EQ(9, world_region.get_region_ids()[2]);

    second_ids.push_back(42);
    world_region.set_region_ids(second_ids);
    FT_ASSERT_EQ(1u, world_region.get_region_ids().size());
    FT_ASSERT_EQ(42, world_region.get_region_ids()[0]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, world_region.get_error());
    return (1);
}

FT_TEST(test_world_region_move_assignment_clears_source, "Game: moving a world_region resets the source")
{
    ft_vector<int> source_ids;
    ft_world_region source_region;
    ft_world_region target_region;

    source_ids.push_back(5);
    source_ids.push_back(8);
    source_region.set_world_id(4);
    source_region.set_region_ids(source_ids);
    target_region.set_world_id(1);
    target_region = ft_move(source_region);
    FT_ASSERT_EQ(4, target_region.get_world_id());
    FT_ASSERT_EQ(2u, target_region.get_region_ids().size());
    FT_ASSERT_EQ(5, target_region.get_region_ids()[0]);
    FT_ASSERT_EQ(8, target_region.get_region_ids()[1]);
    FT_ASSERT_EQ(0, source_region.get_world_id());
    FT_ASSERT_EQ(0u, source_region.get_region_ids().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source_region.get_error());
    return (1);
}

FT_TEST(test_world_region_copy_assignment_preserves_content, "Game: copying world_region retains identifiers")
{
    ft_vector<int> original_ids;
    ft_world_region original_region;
    ft_world_region copied_region;

    original_ids.push_back(15);
    original_ids.push_back(27);
    original_region.set_world_id(6);
    original_region.set_region_ids(original_ids);
    copied_region.set_world_id(2);
    copied_region = original_region;
    FT_ASSERT_EQ(6, copied_region.get_world_id());
    FT_ASSERT_EQ(2u, copied_region.get_region_ids().size());
    FT_ASSERT_EQ(15, copied_region.get_region_ids()[0]);
    FT_ASSERT_EQ(27, copied_region.get_region_ids()[1]);
    FT_ASSERT_EQ(2u, original_region.get_region_ids().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, copied_region.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, original_region.get_error());
    return (1);
}

FT_TEST(test_world_default_region_pointer_available, "Game: world exposes default world_region pointer")
{
    ft_world world;
    ft_sharedptr<ft_world_region> &region_pointer = world.get_world_region();

    FT_ASSERT_EQ(true, static_cast<bool>(region_pointer));
    FT_ASSERT_EQ(0, region_pointer->get_world_id());
    FT_ASSERT_EQ(0u, region_pointer->get_region_ids().size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region_pointer->get_error());
    return (1);
}

FT_TEST(test_world_registry_registers_and_fetches_regions, "Game: registry stores and retrieves world regions")
{
    ft_world_registry registry;
    ft_world_region stored_region;
    ft_world_region fetched_region;
    ft_vector<int> region_ids;

    region_ids.push_back(101);
    region_ids.push_back(202);
    stored_region.set_world_id(3);
    stored_region.set_region_ids(region_ids);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, registry.register_world(stored_region));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, registry.fetch_world(3, fetched_region));
    FT_ASSERT_EQ(3, fetched_region.get_world_id());
    FT_ASSERT_EQ(2u, fetched_region.get_region_ids().size());
    FT_ASSERT_EQ(101, fetched_region.get_region_ids()[0]);
    FT_ASSERT_EQ(202, fetched_region.get_region_ids()[1]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, fetched_region.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, registry.get_error());
    return (1);
}

FT_TEST(test_inventory_add_item_rejects_weight_overflow, "Game: inventory blocks inserts that exceed weight limit")
{
    ft_inventory inventory(0, 5);
    ft_sharedptr<ft_item> heavy(new ft_item());

    heavy->set_item_id(11);
    heavy->set_width(2);
    heavy->set_height(2);
    heavy->set_max_stack(5);
    heavy->set_stack_size(2);
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.add_item(heavy));
    FT_ASSERT_EQ(0, inventory.count_item(11));
    FT_ASSERT_EQ(0, inventory.get_current_weight());
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.get_error());
    return (1);
}

FT_TEST(test_inventory_is_full_checks_capacity, "Game: is_full reflects slot usage when capacity is limited")
{
    ft_inventory inventory(2, 50);
    ft_sharedptr<ft_item> first(new ft_item());
    ft_sharedptr<ft_item> second(new ft_item());
    ft_sharedptr<ft_item> third(new ft_item());

    first->set_item_id(1);
    first->set_width(1);
    first->set_height(1);
    first->set_max_stack(5);
    first->set_stack_size(1);
    second->set_item_id(2);
    second->set_width(1);
    second->set_height(1);
    second->set_max_stack(5);
    second->set_stack_size(1);
    third->set_item_id(3);
    third->set_width(1);
    third->set_height(1);
    third->set_max_stack(5);
    third->set_stack_size(1);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(first));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(second));
    FT_ASSERT_EQ(true, inventory.is_full());
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.add_item(third));
    FT_ASSERT_EQ(2u, inventory.get_used());
    FT_ASSERT_EQ(true, inventory.has_item(1));
    FT_ASSERT_EQ(true, inventory.has_item(2));
    FT_ASSERT_EQ(false, inventory.has_item(3));
    return (1);
}

FT_TEST(test_inventory_partial_stack_before_capacity_error, "Game: adding beyond capacity still merges into existing stack")
{
    ft_inventory inventory(1, 50);
    ft_sharedptr<ft_item> base(new ft_item());
    ft_sharedptr<ft_item> extra(new ft_item());

    base->set_item_id(5);
    base->set_width(1);
    base->set_height(1);
    base->set_max_stack(5);
    base->set_stack_size(3);
    extra->set_item_id(5);
    extra->set_width(1);
    extra->set_height(1);
    extra->set_max_stack(5);
    extra->set_stack_size(4);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, inventory.add_item(base));
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.add_item(extra));
    FT_ASSERT_EQ(5, inventory.count_item(5));
    FT_ASSERT_EQ(5, inventory.get_current_weight());
    FT_ASSERT_EQ(1u, inventory.get_used());
    FT_ASSERT_EQ(FT_ERR_FULL, inventory.get_error());
    return (1);
}

FT_TEST(test_world_region_parameterized_constructor_copies_values, "Game: parameterized world_region copies identifiers")
{
    ft_vector<int> region_ids;
    ft_world_region region;

    region_ids.push_back(4);
    region_ids.push_back(6);
    region_ids.push_back(8);
    region = ft_world_region(12, region_ids);
    region_ids.push_back(10);
    FT_ASSERT_EQ(12, region.get_world_id());
    FT_ASSERT_EQ(3u, region.get_region_ids().size());
    FT_ASSERT_EQ(4, region.get_region_ids()[0]);
    FT_ASSERT_EQ(6, region.get_region_ids()[1]);
    FT_ASSERT_EQ(8, region.get_region_ids()[2]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    return (1);
}

FT_TEST(test_world_region_mutable_ids_reference_updates_state, "Game: mutable get_region_ids enables in-place edits")
{
    ft_world_region region;
    ft_vector<int> &ids = region.get_region_ids();

    ids.push_back(14);
    ids.push_back(16);
    FT_ASSERT_EQ(2u, region.get_region_ids().size());
    FT_ASSERT_EQ(14, region.get_region_ids()[0]);
    FT_ASSERT_EQ(16, region.get_region_ids()[1]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, region.get_error());
    return (1);
}

