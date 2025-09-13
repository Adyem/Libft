#include "character.hpp"
#include "../Errno/errno.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"

json_group *serialize_character(const ft_character &character)
{
    json_group *group = json_create_json_group("character");
    if (!group)
        return (ft_nullptr);
    json_item *item = json_create_item("hit_points", character.get_hit_points());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("physical_armor", character.get_physical_armor());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("magic_armor", character.get_magic_armor());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("current_physical_armor", character.get_current_physical_armor());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("current_magic_armor", character.get_current_magic_armor());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("damage_rule", character.get_damage_rule());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("might", character.get_might());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("agility", character.get_agility());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("endurance", character.get_endurance());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("reason", character.get_reason());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("insigh", character.get_insigh());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("presence", character.get_presence());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("coins", character.get_coins());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("valor", character.get_valor());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("experience", character.get_experience());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("x", character.get_x());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("y", character.get_y());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    item = json_create_item("z", character.get_z());
    if (!item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, item);
    return (group);
}

int deserialize_character(ft_character &character, json_group *group)
{
    json_item *item = json_find_item(group, "hit_points");
    if (item)
        character.set_hit_points(ft_atoi(item->value));
    item = json_find_item(group, "physical_armor");
    if (item)
        character.set_physical_armor(ft_atoi(item->value));
    item = json_find_item(group, "magic_armor");
    if (item)
        character.set_magic_armor(ft_atoi(item->value));
    item = json_find_item(group, "current_physical_armor");
    if (item)
        character.set_current_physical_armor(ft_atoi(item->value));
    item = json_find_item(group, "current_magic_armor");
    if (item)
        character.set_current_magic_armor(ft_atoi(item->value));
    item = json_find_item(group, "damage_rule");
    if (item)
        character.set_damage_rule(static_cast<uint8_t>(ft_atoi(item->value)));
    item = json_find_item(group, "might");
    if (item)
        character.set_might(ft_atoi(item->value));
    item = json_find_item(group, "agility");
    if (item)
        character.set_agility(ft_atoi(item->value));
    item = json_find_item(group, "endurance");
    if (item)
        character.set_endurance(ft_atoi(item->value));
    item = json_find_item(group, "reason");
    if (item)
        character.set_reason(ft_atoi(item->value));
    item = json_find_item(group, "insigh");
    if (item)
        character.set_insigh(ft_atoi(item->value));
    item = json_find_item(group, "presence");
    if (item)
        character.set_presence(ft_atoi(item->value));
    item = json_find_item(group, "coins");
    if (item)
        character.set_coins(ft_atoi(item->value));
    item = json_find_item(group, "valor");
    if (item)
        character.set_valor(ft_atoi(item->value));
    item = json_find_item(group, "experience");
    if (item)
        character.set_experience(ft_atoi(item->value));
    item = json_find_item(group, "x");
    if (item)
        character.set_x(ft_atoi(item->value));
    item = json_find_item(group, "y");
    if (item)
        character.set_y(ft_atoi(item->value));
    item = json_find_item(group, "z");
    if (item)
        character.set_z(ft_atoi(item->value));
    return (ER_SUCCESS);
}

ft_character::ft_character() noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res{0, 0}, _frost_res{0, 0}, _lightning_res{0, 0},
      _air_res{0, 0}, _earth_res{0, 0}, _chaos_res{0, 0},
      _physical_res{0, 0}, _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _error(ER_SUCCESS)
{
    if (this->_buffs.get_error() != ER_SUCCESS)
        this->set_error(this->_buffs.get_error());
    else if (this->_debuffs.get_error() != ER_SUCCESS)
        this->set_error(this->_debuffs.get_error());
    else if (this->_upgrades.get_error() != ER_SUCCESS)
        this->set_error(this->_upgrades.get_error());
    else if (this->_quests.get_error() != ER_SUCCESS)
        this->set_error(this->_quests.get_error());
    else if (this->_achievements.get_error() != ER_SUCCESS)
        this->set_error(this->_achievements.get_error());
    else if (this->_reputation.get_error() != ER_SUCCESS)
        this->set_error(this->_reputation.get_error());
    return ;
}

int ft_character::get_hit_points() const noexcept
{
    return (this->_hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    this->_hit_points = hp;
    return ;
}

bool ft_character::is_alive() const noexcept
{
    return (this->_hit_points > 0);
}

int ft_character::get_physical_armor() const noexcept
{
    return (this->_physical_armor);
}

void ft_character::set_physical_armor(int armor) noexcept
{
    this->_physical_armor = armor;
    this->_current_physical_armor = armor;
    this->_physical_damage_multiplier = 1.0;
    int i = 0;
    while (i < armor)
    {
        this->_physical_damage_multiplier = this->_physical_damage_multiplier * FT_ARMOR_POINT_REDUCTION;
        i++;
    }
    return ;
}

int ft_character::get_magic_armor() const noexcept
{
    return (this->_magic_armor);
}

void ft_character::set_magic_armor(int armor) noexcept
{
    this->_magic_armor = armor;
    this->_current_magic_armor = armor;
    this->_magic_damage_multiplier = 1.0;
    int i = 0;
    while (i < armor)
    {
        this->_magic_damage_multiplier = this->_magic_damage_multiplier * FT_ARMOR_POINT_REDUCTION;
        i++;
    }
    return ;
}

int ft_character::get_current_physical_armor() const noexcept
{
    return (this->_current_physical_armor);
}

void ft_character::set_current_physical_armor(int armor) noexcept
{
    this->_current_physical_armor = armor;
    return ;
}

int ft_character::get_current_magic_armor() const noexcept
{
    return (this->_current_magic_armor);
}

void ft_character::set_current_magic_armor(int armor) noexcept
{
    this->_current_magic_armor = armor;
    return ;
}

void ft_character::restore_physical_armor() noexcept
{
    this->_current_physical_armor = this->_physical_armor;
    return ;
}

void ft_character::restore_magic_armor() noexcept
{
    this->_current_magic_armor = this->_magic_armor;
    return ;
}

void ft_character::restore_armor() noexcept
{
    this->restore_physical_armor();
    this->restore_magic_armor();
    return ;
}

void ft_character::set_damage_rule(uint8_t rule) noexcept
{
    this->_damage_rule = rule;
    return ;
}

uint8_t ft_character::get_damage_rule() const noexcept
{
    return (this->_damage_rule);
}

void ft_character::take_damage(long long damage, uint8_t type) noexcept
{
    if (this->_damage_rule == FT_DAMAGE_RULE_FLAT)
        this->take_damage_flat(damage, type);
    else if (this->_damage_rule == FT_DAMAGE_RULE_SCALED)
        this->take_damage_scaled(damage, type);
    else if (this->_damage_rule == FT_DAMAGE_RULE_BUFFER)
        this->take_damage_buffer(damage, type);
    return ;
}

void ft_character::take_damage_flat(long long damage, uint8_t type) noexcept
{
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
        damage = damage - this->_physical_armor;
    else if (type == FT_DAMAGE_MAGICAL)
        damage = damage - this->_magic_armor;
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    return ;
}

void ft_character::take_damage_scaled(long long damage, uint8_t type) noexcept
{
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
    {
        double scaled = static_cast<double>(damage) * this->_physical_damage_multiplier;
        damage = static_cast<long long>(scaled);
    }
    else if (type == FT_DAMAGE_MAGICAL)
    {
        double scaled = static_cast<double>(damage) * this->_magic_damage_multiplier;
        damage = static_cast<long long>(scaled);
    }
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    return ;
}

void ft_character::take_damage_buffer(long long damage, uint8_t type) noexcept
{
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
    {
        if (this->_current_physical_armor > 0)
        {
            if (damage <= this->_current_physical_armor)
            {
                this->_current_physical_armor = this->_current_physical_armor - static_cast<int>(damage);
                damage = 0;
            }
            else
            {
                damage = damage - this->_current_physical_armor;
                this->_current_physical_armor = 0;
            }
        }
    }
    else if (type == FT_DAMAGE_MAGICAL)
    {
        if (this->_current_magic_armor > 0)
        {
            if (damage <= this->_current_magic_armor)
            {
                this->_current_magic_armor = this->_current_magic_armor - static_cast<int>(damage);
                damage = 0;
            }
            else
            {
                damage = damage - this->_current_magic_armor;
                this->_current_magic_armor = 0;
            }
        }
    }
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    return ;
}

int ft_character::get_might() const noexcept
{
    return (this->_might);
}

void ft_character::set_might(int might) noexcept
{
    this->_might = might;
    return ;
}

int ft_character::get_agility() const noexcept
{
    return (this->_agility);
}

void ft_character::set_agility(int agility) noexcept
{
    this->_agility = agility;
    return ;
}

int ft_character::get_endurance() const noexcept
{
    return (this->_endurance);
}

void ft_character::set_endurance(int endurance) noexcept
{
    this->_endurance = endurance;
    return ;
}

int ft_character::get_reason() const noexcept
{
    return (this->_reason);
}

void ft_character::set_reason(int reason) noexcept
{
    this->_reason = reason;
    return ;
}

int ft_character::get_insigh() const noexcept
{
    return (this->_insigh);
}

void ft_character::set_insigh(int insigh) noexcept
{
    this->_insigh = insigh;
    return ;
}

int ft_character::get_presence() const noexcept
{
    return (this->_presence);
}

void ft_character::set_presence(int presence) noexcept
{
    this->_presence = presence;
    return ;
}

int ft_character::get_coins() const noexcept
{
    return (this->_coins);
}

void ft_character::set_coins(int coins) noexcept
{
    this->_coins = coins;
    return ;
}

void ft_character::add_coins(int coins) noexcept
{
    this->_coins += coins;
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    this->_coins -= coins;
    return ;
}

int ft_character::get_valor() const noexcept
{
    return (this->_valor);
}

void ft_character::set_valor(int valor) noexcept
{
    this->_valor = valor;
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    this->_valor += valor;
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    this->_valor -= valor;
    return ;
}

int ft_character::get_experience() const noexcept
{
    return (this->_experience);
}

void ft_character::set_experience(int experience) noexcept
{
    this->_experience = experience;
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    this->_experience += experience;
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    this->_experience -= experience;
    return ;
}

int ft_character::get_x() const noexcept
{
    return (this->_x);
}

void ft_character::set_x(int x) noexcept
{
    this->_x = x;
    return ;
}

int ft_character::get_y() const noexcept
{
    return (this->_y);
}

void ft_character::set_y(int y) noexcept
{
    this->_y = y;
    return ;
}

int ft_character::get_z() const noexcept
{
    return (this->_z);
}

void ft_character::set_z(int z) noexcept
{
    this->_z = z;
    return ;
}

void ft_character::move(int dx, int dy, int dz) noexcept
{
    this->_x += dx;
    this->_y += dy;
    this->_z += dz;
    return ;
}

ft_resistance ft_character::get_fire_res() const noexcept
{
    return (this->_fire_res);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    this->_fire_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_frost_res() const noexcept
{
    return (this->_frost_res);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    this->_frost_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_lightning_res() const noexcept
{
    return (this->_lightning_res);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    this->_lightning_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_air_res() const noexcept
{
    return (this->_air_res);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    this->_air_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_earth_res() const noexcept
{
    return (this->_earth_res);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    this->_earth_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_chaos_res() const noexcept
{
    return (this->_chaos_res);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    this->_chaos_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_physical_res() const noexcept
{
    return (this->_physical_res);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    this->_physical_res = {percent, flat};
    return ;
}

ft_map<int, ft_buff> &ft_character::get_buffs() noexcept
{
    return (this->_buffs);
}

const ft_map<int, ft_buff> &ft_character::get_buffs() const noexcept
{
    return (this->_buffs);
}

ft_map<int, ft_debuff> &ft_character::get_debuffs() noexcept
{
    return (this->_debuffs);
}

const ft_map<int, ft_debuff> &ft_character::get_debuffs() const noexcept
{
    return (this->_debuffs);
}

ft_map<int, ft_upgrade> &ft_character::get_upgrades() noexcept
{
    return (this->_upgrades);
}

const ft_map<int, ft_upgrade> &ft_character::get_upgrades() const noexcept
{
    return (this->_upgrades);
}


ft_map<int, ft_quest> &ft_character::get_quests() noexcept
{
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_character::get_quests() const noexcept
{
    return (this->_quests);
}

ft_map<int, ft_achievement> &ft_character::get_achievements() noexcept
{
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_character::get_achievements() const noexcept
{
    return (this->_achievements);
}

ft_reputation &ft_character::get_reputation() noexcept
{
    return (this->_reputation);
}

const ft_reputation &ft_character::get_reputation() const noexcept
{
    return (this->_reputation);
}

ft_experience_table &ft_character::get_experience_table() noexcept
{
    return (this->_experience_table);
}

const ft_experience_table &ft_character::get_experience_table() const noexcept
{
    return (this->_experience_table);
}

int ft_character::get_level() const noexcept
{
    return (this->_experience_table.get_level(this->_experience));
}

void ft_character::apply_modifier(const ft_item_modifier &mod, int sign) noexcept
{
    if (mod.id == 1)
        this->set_physical_armor(this->_physical_armor + mod.value * sign);
    else if (mod.id == 9)
        this->set_magic_armor(this->_magic_armor + mod.value * sign);
    else if (mod.id == 2)
        this->_might += mod.value * sign;
    else if (mod.id == 3)
        this->_agility += mod.value * sign;
    else if (mod.id == 4)
        this->_endurance += mod.value * sign;
    else if (mod.id == 5)
        this->_reason += mod.value * sign;
    else if (mod.id == 6)
        this->_insigh += mod.value * sign;
    else if (mod.id == 7)
        this->_presence += mod.value * sign;
    else if (mod.id == 8)
        this->_hit_points += mod.value * sign;
    return ;
}

int ft_character::equip_item(int slot, const ft_item &item) noexcept
{
    ft_item *current = this->_equipment.get_item(slot);
    if (current)
    {
        this->apply_modifier(current->get_modifier1(), -1);
        this->apply_modifier(current->get_modifier2(), -1);
        this->apply_modifier(current->get_modifier3(), -1);
        this->apply_modifier(current->get_modifier4(), -1);
    }
    if (this->_equipment.equip(slot, item) != ER_SUCCESS)
    {
        this->set_error(this->_equipment.get_error());
        return (this->_error);
    }
    this->apply_modifier(item.get_modifier1(), 1);
    this->apply_modifier(item.get_modifier2(), 1);
    this->apply_modifier(item.get_modifier3(), 1);
    this->apply_modifier(item.get_modifier4(), 1);
    return (ER_SUCCESS);
}

void ft_character::unequip_item(int slot) noexcept
{
    ft_item *item = this->_equipment.get_item(slot);
    if (item)
    {
        this->apply_modifier(item->get_modifier1(), -1);
        this->apply_modifier(item->get_modifier2(), -1);
        this->apply_modifier(item->get_modifier3(), -1);
        this->apply_modifier(item->get_modifier4(), -1);
    }
    this->_equipment.unequip(slot);
    return ;
}

ft_item *ft_character::get_equipped_item(int slot) noexcept
{
    return (this->_equipment.get_item(slot));
}

const ft_item *ft_character::get_equipped_item(int slot) const noexcept
{
    return (this->_equipment.get_item(slot));
}

int ft_character::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_character::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_character::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
