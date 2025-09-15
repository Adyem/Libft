#include "../../Game/game_character.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_debuff.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_reputation.hpp"
#include "../../Game/game_map3d.hpp"
#include "../../Game/game_item.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_inventory.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../JSon/json.hpp"
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
    if (!uentry)
        return (0);
    hero.set_might(hero.get_might() + uentry->value.get_modifier1());
    if (hero.get_might() != 18)
        return (0);

    ft_quest quest;
    quest.set_id(1);
    quest.set_phases(2);
    hero.get_quests().insert(quest.get_id(), quest);
    Pair<int, ft_quest>* qentry = hero.get_quests().find(1);
    if (!qentry)
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
    if (pack.add_item(potion) != ER_SUCCESS)
        return (0);
    ft_sharedptr<ft_item> more(new ft_item());
    more->set_item_id(1);
    more->set_max_stack(10);
    more->set_stack_size(3);
    pack.add_item(more);
    Pair<int, ft_sharedptr<ft_item> > *ientry = pack.get_items().find(0);
    if (!ientry || ientry->value->get_stack_size() != 8)
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

int test_inventory_slots(void)
{
    ft_inventory inventory(4);
    ft_sharedptr<ft_item> bulky(new ft_item());
    bulky->set_item_id(1);
    bulky->set_max_stack(1);
    bulky->set_stack_size(1);
    bulky->set_width(2);
    bulky->set_height(2);
    if (inventory.add_item(bulky) != ER_SUCCESS)
        return (0);
    if (inventory.get_used() != 4)
        return (0);
    ft_sharedptr<ft_item> small(new ft_item());
    small->set_item_id(2);
    small->set_max_stack(1);
    small->set_stack_size(1);
    if (inventory.add_item(small) != CHARACTER_INVENTORY_FULL)
        return (0);
    return (1);
}

int test_inventory_count(void)
{
    ft_inventory inv(5);
    ft_sharedptr<ft_item> potion(new ft_item());
    potion->set_item_id(1);
    potion->set_max_stack(10);
    potion->set_stack_size(7);
    inv.add_item(potion);

    ft_sharedptr<ft_item> more(new ft_item());
    more->set_item_id(1);
    more->set_max_stack(10);
    more->set_stack_size(4);
    inv.add_item(more);

    if (!inv.has_item(1) || inv.count_item(1) != 11)
        return (0);
    if (inv.has_item(2) || inv.count_item(2) != 0)
        return (0);
    return (1);
}

int test_inventory_full(void)
{
    ft_inventory inv(1);
    ft_sharedptr<ft_item> item(new ft_item());
    item->set_item_id(1);
    item->set_max_stack(5);
    item->set_stack_size(5);
    if (inv.is_full())
        return (0);
    if (inv.add_item(item) != ER_SUCCESS)
        return (0);
    if (!inv.is_full())
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
    if (world.save_to_file("test_save.json", hero, inventory) != ER_SUCCESS)
        return (0);
    ft_character loaded_hero;
    ft_world loaded_world;
    ft_inventory loaded_inventory;
    if (loaded_world.load_from_file("test_save.json", loaded_hero, loaded_inventory) != ER_SUCCESS)
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

int test_reputation_subtracters(void)
{
    ft_reputation rep;
    rep.set_total_rep(20);
    rep.sub_total_rep(5);
    rep.set_current_rep(10);
    rep.sub_current_rep(3);
    return (rep.get_total_rep() == 12 && rep.get_current_rep() == 7);
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
