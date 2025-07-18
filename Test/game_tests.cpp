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
