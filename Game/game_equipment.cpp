#include "game_equipment.hpp"
#include "../Libft/libft.hpp"

ft_equipment::ft_equipment() noexcept
    : _head(), _chest(), _weapon(),
      _has_head(false), _has_chest(false), _has_weapon(false),
      _error(ER_SUCCESS)
{
    return ;
}

ft_equipment::ft_equipment(const ft_equipment &other) noexcept
    : _head(other._head), _chest(other._chest), _weapon(other._weapon),
      _has_head(other._has_head), _has_chest(other._has_chest), _has_weapon(other._has_weapon),
      _error(other._error)
{
    return ;
}

ft_equipment &ft_equipment::operator=(const ft_equipment &other) noexcept
{
    if (this != &other)
    {
        this->_head = other._head;
        this->_chest = other._chest;
        this->_weapon = other._weapon;
        this->_has_head = other._has_head;
        this->_has_chest = other._has_chest;
        this->_has_weapon = other._has_weapon;
        this->_error = other._error;
    }
    return (*this);
}

ft_equipment::ft_equipment(ft_equipment &&other) noexcept
    : _head(ft_move(other._head)), _chest(ft_move(other._chest)), _weapon(ft_move(other._weapon)),
      _has_head(other._has_head), _has_chest(other._has_chest), _has_weapon(other._has_weapon),
      _error(other._error)
{
    other._has_head = false;
    other._has_chest = false;
    other._has_weapon = false;
    other._error = ER_SUCCESS;
    return ;
}

ft_equipment &ft_equipment::operator=(ft_equipment &&other) noexcept
{
    if (this != &other)
    {
        this->_head = ft_move(other._head);
        this->_chest = ft_move(other._chest);
        this->_weapon = ft_move(other._weapon);
        this->_has_head = other._has_head;
        this->_has_chest = other._has_chest;
        this->_has_weapon = other._has_weapon;
        this->_error = other._error;
        other._has_head = false;
        other._has_chest = false;
        other._has_weapon = false;
        other._error = ER_SUCCESS;
    }
    return (*this);
}

void ft_equipment::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

int ft_equipment::equip(int slot, const ft_item &item) noexcept
{
    this->_error = ER_SUCCESS;
    if (slot == EQUIP_HEAD)
    {
        this->_head = item;
        this->_has_head = true;
    }
    else if (slot == EQUIP_CHEST)
    {
        this->_chest = item;
        this->_has_chest = true;
    }
    else if (slot == EQUIP_WEAPON)
    {
        this->_weapon = item;
        this->_has_weapon = true;
    }
    else
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    return (ER_SUCCESS);
}

void ft_equipment::unequip(int slot) noexcept
{
    if (slot == EQUIP_HEAD)
        this->_has_head = false;
    else if (slot == EQUIP_CHEST)
        this->_has_chest = false;
    else if (slot == EQUIP_WEAPON)
        this->_has_weapon = false;
    else
        this->set_error(GAME_GENERAL_ERROR);
    return ;
}

ft_item *ft_equipment::get_item(int slot) noexcept
{
    if (slot == EQUIP_HEAD)
    {
        if (this->_has_head)
            return (&this->_head);
        return (ft_nullptr);
    }
    else if (slot == EQUIP_CHEST)
    {
        if (this->_has_chest)
            return (&this->_chest);
        return (ft_nullptr);
    }
    else if (slot == EQUIP_WEAPON)
    {
        if (this->_has_weapon)
            return (&this->_weapon);
        return (ft_nullptr);
    }
    this->set_error(GAME_GENERAL_ERROR);
    return (ft_nullptr);
}

const ft_item *ft_equipment::get_item(int slot) const noexcept
{
    if (slot == EQUIP_HEAD)
    {
        if (this->_has_head)
            return (&this->_head);
        return (ft_nullptr);
    }
    else if (slot == EQUIP_CHEST)
    {
        if (this->_has_chest)
            return (&this->_chest);
        return (ft_nullptr);
    }
    else if (slot == EQUIP_WEAPON)
    {
        if (this->_has_weapon)
            return (&this->_weapon);
        return (ft_nullptr);
    }
    this->set_error(GAME_GENERAL_ERROR);
    return (ft_nullptr);
}

int ft_equipment::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_equipment::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}
