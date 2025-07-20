#include "../Game/character.hpp"
#include "../Game/buff.hpp"
#include "../Game/debuff.hpp"
#include "../Game/quest.hpp"
#include "../Game/reputation.hpp"
#include "../Game/map3d.hpp"
#include "../Game/item.hpp"
#include "../Game/upgrade.hpp"
#include "../Game/world.hpp"
#include "../Game/event.hpp"
#include "../Game/inventory.hpp"
#include "../Errno/errno.hpp"

int test_game_simulation(void)
{
    ft_character hero;
    hero.set_hit_points(50);
    hero.set_might(10);
    hero.set_armor(5);

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
        return 0;

    ft_debuff weakness;
    weakness.set_id(2);
    weakness.set_modifier1(-2);
    hero.get_debuffs().insert(weakness.get_id(), weakness);
    hero.set_armor(hero.get_armor() + weakness.get_modifier1());
    if (hero.get_armor() != 3)
        return 0;

    ft_upgrade upgrade;
    upgrade.set_id(1);
    upgrade.set_modifier1(3);
    hero.get_upgrades().insert(upgrade.get_id(), upgrade);
    Pair<int, ft_upgrade>* uentry = hero.get_upgrades().find(1);
    if (!uentry)
        return 0;
    hero.set_might(hero.get_might() + uentry->value.get_modifier1());
    if (hero.get_might() != 18)
        return 0;

    ft_quest quest;
    quest.set_id(1);
    quest.set_phases(2);
    hero.get_quests().insert(quest.get_id(), quest);
    Pair<int, ft_quest>* qentry = hero.get_quests().find(1);
    if (!qentry)
        return 0;
    qentry->value.set_current_phase(1);
    if (qentry->value.get_current_phase() != 1)
        return 0;

    hero.get_reputation().set_milestone(1, 10);
    hero.get_reputation().add_current_rep(4);
    if (hero.get_reputation().get_current_rep() != 4 ||
        hero.get_reputation().get_total_rep() != 4)
        return 0;

    ft_world overworld;
    ft_event meeting;
    meeting.set_id(1);
    meeting.set_duration(5);
    overworld.get_events().insert(meeting.get_id(), meeting);
    Pair<int, ft_event>* eentry = overworld.get_events().find(1);
    if (!eentry || eentry->value.get_duration() != 5)
        return 0;

    ft_inventory pack(2);
    ft_item potion;
    potion.set_item_id(1);
    potion.set_max_stack(10);
    potion.set_current_stack(5);
    if (pack.add_item(potion) != ER_SUCCESS)
        return 0;
    ft_item more;
    more.set_item_id(1);
    more.set_max_stack(10);
    more.set_current_stack(3);
    pack.add_item(more);
    Pair<int, ft_item>* ientry = pack.get_items().find(0);
    if (!ientry || ientry->value.get_current_stack() != 8)
        return 0;

    if (grid.get(hero.get_x(), hero.get_y(), hero.get_z()) != 1)
        return 0;

    return 1;
}


int test_item_basic(void)
{
    ft_item item;
    item.set_item_id(1);
    item.set_max_stack(10);
    item.set_current_stack(3);
    item.set_modifier1_id(5);
    item.set_modifier1_value(2);
    if (item.get_item_id() != 1 || item.get_max_stack() != 10 ||
        item.get_current_stack() != 3 || item.get_modifier1_id() != 5 ||
        item.get_modifier1_value() != 2)
        return 0;
    return 1;
}

int test_inventory_count(void)
{
    ft_inventory inv(5);
    ft_item potion;
    potion.set_item_id(1);
    potion.set_max_stack(10);
    potion.set_current_stack(7);
    inv.add_item(potion);

    ft_item more;
    more.set_item_id(1);
    more.set_max_stack(10);
    more.set_current_stack(4);
    inv.add_item(more);

    if (!inv.has_item(1) || inv.count_item(1) != 11)
        return 0;
    if (inv.has_item(2) || inv.count_item(2) != 0)
        return 0;
    return 1;
}

int test_inventory_full(void)
{
    ft_inventory inv(1);
    ft_item item;
    item.set_item_id(1);
    item.set_max_stack(5);
    item.set_current_stack(5);
    if (inv.is_full())
        return 0;
    if (inv.add_item(item) != ER_SUCCESS)
        return 0;
    if (!inv.is_full())
        return 0;
    return 1;
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
    item.set_current_stack(7);
    item.sub_from_stack(3);
    return (item.get_current_stack() == 4);
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
        return 0;
    q.advance_phase();
    if (q.get_current_phase() != 1 || q.is_complete())
        return 0;
    q.advance_phase();
    q.advance_phase();
    if (!q.is_complete() || q.get_current_phase() != 3)
        return 0;
    q.advance_phase();
    return (q.get_current_phase() == 3);
}
