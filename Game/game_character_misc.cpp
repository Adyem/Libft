#include "game_character.hpp"

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
    damage = this->apply_skill_modifiers(damage);
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
    damage = this->apply_skill_modifiers(damage);
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
    damage = this->apply_skill_modifiers(damage);
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
void ft_character::move(int dx, int dy, int dz) noexcept
{
    this->_x += dx;
    this->_y += dy;
    this->_z += dz;
    return ;
}
long long ft_character::apply_skill_modifiers(long long damage) const noexcept
{
    const Pair<int, ft_skill> *skill_ptr = this->_skills.end() - this->_skills.size();
    const Pair<int, ft_skill> *skill_end = this->_skills.end();
    while (skill_ptr != skill_end)
    {
        if (skill_ptr->value.get_cooldown() == 0)
        {
            damage += skill_ptr->value.get_modifier1();
            int percent = skill_ptr->value.get_modifier2();
            if (percent != 0)
            {
                long long delta = damage * percent;
                damage += delta / 100;
            }
        }
        ++skill_ptr;
    }
    if (damage < 0)
        damage = 0;
    return (damage);
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
