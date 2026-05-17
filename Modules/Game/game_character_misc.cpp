#include "game_character.hpp"
#include "../Errno/errno.hpp"

void game_character::restore_physical_armor() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->restore_physical_armor_internal();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::restore_magic_armor() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->restore_magic_armor_internal();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::restore_armor() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->restore_physical_armor_internal();
    this->restore_magic_armor_internal();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage(int64_t damage, uint8_t type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_damage_rule == FT_DAMAGE_RULE_FLAT)
        this->take_damage_flat_internal(damage, type);
    else if (this->_damage_rule == FT_DAMAGE_RULE_SCALED)
        this->take_damage_scaled_internal(damage, type);
    else if (this->_damage_rule == FT_DAMAGE_RULE_BUFFER)
        this->take_damage_buffer_internal(damage, type);
    else if (this->_damage_rule == FT_DAMAGE_RULE_MAGIC_SHIELD)
        this->take_damage_magic_shield_internal(damage, type);
    else
        this->take_damage_flat_internal(damage, type);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage_flat(int64_t damage, uint8_t type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->take_damage_flat_internal(damage, type);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage_scaled(int64_t damage, uint8_t type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->take_damage_scaled_internal(damage, type);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage_buffer(int64_t damage, uint8_t type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->take_damage_buffer_internal(damage, type);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage_magic_shield(int64_t damage, uint8_t type) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->take_damage_magic_shield_internal(damage, type);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::take_damage_flat_internal(int64_t damage, uint8_t type) noexcept
{
    int32_t previous_hit_points;
    int64_t damage_applied;

    previous_hit_points = this->_hit_points;
    damage = this->apply_skill_modifiers_internal(damage);
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
        damage = damage - this->_physical_armor;
    else if (type == FT_DAMAGE_MAGICAL)
        damage = damage - this->_magic_armor;
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int32_t>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    damage_applied = static_cast<int64_t>(previous_hit_points - this->_hit_points);
    if (damage_applied < 0)
        damage_applied = 0;
    this->emit_game_metric("character.damage_taken", "hit_points",
        damage_applied, this->_hit_points, "hp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_character::take_damage_scaled_internal(int64_t damage, uint8_t type) noexcept
{
    int32_t previous_hit_points;
    int64_t damage_applied;

    previous_hit_points = this->_hit_points;
    damage = this->apply_skill_modifiers_internal(damage);
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
    {
        double scaled = static_cast<double>(damage) * this->_physical_damage_multiplier;
        damage = static_cast<int64_t>(scaled);
    }
    else if (type == FT_DAMAGE_MAGICAL)
    {
        double scaled = static_cast<double>(damage) * this->_magic_damage_multiplier;
        damage = static_cast<int64_t>(scaled);
    }
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int32_t>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    damage_applied = static_cast<int64_t>(previous_hit_points - this->_hit_points);
    if (damage_applied < 0)
        damage_applied = 0;
    this->emit_game_metric("character.damage_taken", "hit_points",
        damage_applied, this->_hit_points, "hp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_character::take_damage_buffer_internal(int64_t damage, uint8_t type) noexcept
{
    int32_t previous_hit_points;
    int64_t damage_applied;

    previous_hit_points = this->_hit_points;
    damage = this->apply_skill_modifiers_internal(damage);
    if (damage < 0)
        damage = 0;
    if (type == FT_DAMAGE_PHYSICAL)
    {
        if (this->_current_physical_armor > 0)
        {
            if (damage <= this->_current_physical_armor)
            {
                this->_current_physical_armor = this->_current_physical_armor - static_cast<int32_t>(damage);
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
                this->_current_magic_armor = this->_current_magic_armor - static_cast<int32_t>(damage);
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
    this->_hit_points = this->_hit_points - static_cast<int32_t>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    damage_applied = static_cast<int64_t>(previous_hit_points - this->_hit_points);
    if (damage_applied < 0)
        damage_applied = 0;
    this->emit_game_metric("character.damage_taken", "hit_points",
        damage_applied, this->_hit_points, "hp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_character::take_damage_magic_shield_internal(int64_t damage, uint8_t type) noexcept
{
    int32_t previous_hit_points;
    int64_t damage_applied;

    previous_hit_points = this->_hit_points;
    damage = this->apply_skill_modifiers_internal(damage);
    if (damage < 0)
        damage = 0;
    if (this->_current_magic_armor > 0)
    {
        if (damage <= this->_current_magic_armor)
        {
            this->_current_magic_armor = this->_current_magic_armor - static_cast<int32_t>(damage);
            damage = 0;
        }
        else
        {
            damage = damage - this->_current_magic_armor;
            this->_current_magic_armor = 0;
        }
    }
    if (type == FT_DAMAGE_PHYSICAL)
    {
#if FT_PHYSICAL_DAMAGE_REDUCTION == FT_DAMAGE_RULE_FLAT
        damage = damage - this->_physical_armor;
#elif FT_PHYSICAL_DAMAGE_REDUCTION == FT_DAMAGE_RULE_SCALED
        double scaled = static_cast<double>(damage) * this->_physical_damage_multiplier;
        damage = static_cast<int64_t>(scaled);
#endif
    }
    else if (type == FT_DAMAGE_MAGICAL)
    {
#if FT_MAGIC_DAMAGE_REDUCTION == FT_DAMAGE_RULE_FLAT
        damage = damage - this->_magic_armor;
#elif FT_MAGIC_DAMAGE_REDUCTION == FT_DAMAGE_RULE_SCALED
        double scaled = static_cast<double>(damage) * this->_magic_damage_multiplier;
        damage = static_cast<int64_t>(scaled);
#endif
    }
    if (damage < 0)
        damage = 0;
    this->_hit_points = this->_hit_points - static_cast<int32_t>(damage);
    if (this->_hit_points < 0)
        this->_hit_points = 0;
    damage_applied = static_cast<int64_t>(previous_hit_points - this->_hit_points);
    if (damage_applied < 0)
        damage_applied = 0;
    this->emit_game_metric("character.damage_taken", "hit_points",
        damage_applied, this->_hit_points, "hp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_character::move(int32_t dx, int32_t dy, int32_t dz) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_x += dx;
    this->_y += dy;
    this->_z += dz;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int64_t game_character::apply_skill_modifiers(int64_t damage) const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t result;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_character *>(this)->set_error(lock_error);
        return (0);
    }
    result = this->apply_skill_modifiers_internal(damage);
    const_cast<game_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

int64_t game_character::apply_skill_modifiers_internal(int64_t damage) const noexcept
{
    const Pair<int32_t, game_skill> *skill_ptr;
    const Pair<int32_t, game_skill> *skill_end;

    skill_end = this->_skills.end();
    skill_ptr = skill_end - this->_skills.size();
    while (skill_ptr != skill_end)
    {
        if (skill_ptr->value.get_cooldown() == 0)
        {
            damage += skill_ptr->value.get_modifier1();
            int32_t percent = skill_ptr->value.get_modifier2();
            if (percent != 0)
            {
                int64_t delta = damage * percent;
                damage += delta / 100;
            }
        }
        ++skill_ptr;
    }
    if (damage < 0)
        damage = 0;
    return (damage);
}

void game_character::apply_modifier(const game_item_modifier &mod, int32_t sign) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->apply_modifier_internal(mod, sign);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void game_character::apply_modifier_internal(const game_item_modifier &mod, int32_t sign) noexcept
{
    int32_t modifier_error;
    int32_t modifier_identifier;
    int32_t modifier_value;

    modifier_error = mod.get_error();
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    modifier_identifier = mod.get_id();
    modifier_error = mod.get_error();
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    modifier_value = mod.get_value();
    modifier_error = mod.get_error();
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    if (modifier_identifier == 1)
        this->set_physical_armor_internal(this->_physical_armor + modifier_value * sign);
    else if (modifier_identifier == 9)
        this->set_magic_armor_internal(this->_magic_armor + modifier_value * sign);
    else if (modifier_identifier == 2)
        this->_might += modifier_value * sign;
    else if (modifier_identifier == 3)
        this->_agility += modifier_value * sign;
    else if (modifier_identifier == 4)
        this->_endurance += modifier_value * sign;
    else if (modifier_identifier == 5)
        this->_reason += modifier_value * sign;
    else if (modifier_identifier == 6)
        this->_insigh += modifier_value * sign;
    else if (modifier_identifier == 7)
        this->_presence += modifier_value * sign;
    else if (modifier_identifier == 8)
        this->_hit_points += modifier_value * sign;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_character::equip_item(int32_t slot, const ft_sharedptr<game_item> &item) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_sharedptr<game_item> *current;
    int32_t equip_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->get_error());
    }
    current = this->_equipment.get_item(slot);
    equip_error = this->_equipment.get_error();
    if (this->handle_component_error(equip_error) == FT_TRUE)
    {
        if (current != ft_nullptr)
        {
            (void)current->destroy();
            delete current;
        }
        this->unlock_internal(lock_acquired);
        return (this->get_error());
    }
    equip_error = this->_equipment.equip(slot, item);
    if (equip_error != FT_ERR_SUCCESS)
    {
        this->handle_component_error(equip_error);
        if (current != ft_nullptr)
        {
            (void)current->destroy();
            delete current;
        }
        this->unlock_internal(lock_acquired);
        return (this->get_error());
    }
    if (current != ft_nullptr && *current)
    {
        game_item_modifier mod;
        if ((*current)->get_modifier1(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*current)->get_modifier2(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*current)->get_modifier3(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*current)->get_modifier4(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
    }
    if (current != ft_nullptr)
    {
        (void)current->destroy();
        delete current;
    }
    if (item)
    {
        game_item_modifier mod;
        if (item->get_modifier1(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, 1);
        if (item->get_modifier2(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, 1);
        if (item->get_modifier3(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, 1);
        if (item->get_modifier4(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, 1);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void game_character::unequip_item(int32_t slot) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_sharedptr<game_item> *item;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    item = this->_equipment.get_item(slot);
    if (item != ft_nullptr && *item)
    {
        game_item_modifier mod;
        if ((*item)->get_modifier1(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*item)->get_modifier2(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*item)->get_modifier3(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
        if ((*item)->get_modifier4(mod) == FT_ERR_SUCCESS)
            this->apply_modifier_internal(mod, -1);
    }
    if (item != ft_nullptr)
    {
        (void)item->destroy();
        delete item;
    }
    this->_equipment.unequip(slot);
    if (this->handle_component_error(this->_equipment.get_error()) == FT_TRUE)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}
