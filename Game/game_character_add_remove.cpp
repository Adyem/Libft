#include "game_character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int32_t coins) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_coins);
    this->_coins += coins;
    delta_value = static_cast<int64_t>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::sub_coins(int32_t coins) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_coins);
    this->_coins -= coins;
    delta_value = static_cast<int64_t>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::add_valor(int32_t valor) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_valor);
    this->_valor += valor;
    delta_value = static_cast<int64_t>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::sub_valor(int32_t valor) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_valor);
    this->_valor -= valor;
    delta_value = static_cast<int64_t>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::add_experience(int32_t experience) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_experience);
    this->_experience += experience;
    delta_value = static_cast<int64_t>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::sub_experience(int32_t experience) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int64_t previous_total;
    int64_t delta_value;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    previous_total = static_cast<int64_t>(this->_experience);
    this->_experience -= experience;
    delta_value = static_cast<int64_t>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int32_t ft_character::add_skill(const ft_skill &skill) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->get_error());
    }
    this->_skills.insert(skill.get_id(), skill);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_character::remove_skill(int32_t id) noexcept
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
    this->_skills.remove(id);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}
