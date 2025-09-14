#include "game_equipment.hpp"
#include "../Libft/libft.hpp"

ft_equipment::ft_equipment() noexcept
    : _head(ft_nullptr), _chest(ft_nullptr), _weapon(ft_nullptr),
      _error_code(ER_SUCCESS)
{
    return ;
}

ft_equipment::ft_equipment(const ft_equipment &other) noexcept
    : _head(other._head), _chest(other._chest), _weapon(other._weapon),
      _error_code(other._error_code)
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
        this->_error_code = other._error_code;
    }
    return (*this);
}

ft_equipment::ft_equipment(ft_equipment &&other) noexcept
    : _head(ft_move(other._head)), _chest(ft_move(other._chest)), _weapon(ft_move(other._weapon)),
      _error_code(other._error_code)
{
    other._error_code = ER_SUCCESS;
    return ;
}

ft_equipment &ft_equipment::operator=(ft_equipment &&other) noexcept
{
    if (this != &other)
    {
        this->_head = ft_move(other._head);
        this->_chest = ft_move(other._chest);
        this->_weapon = ft_move(other._weapon);
        this->_error_code = other._error_code;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

void ft_equipment::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error_code = err;
    return ;
}

int ft_equipment::equip(int slot, const ft_sharedptr<ft_item> &item) noexcept
{
    this->_error_code = ER_SUCCESS;
    if (!item)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error_code);
    }
    if (item.get_error() != ER_SUCCESS)
    {
        this->set_error(item.get_error());
        return (this->_error_code);
    }
    if (item->get_error() != ER_SUCCESS)
    {
        this->set_error(item->get_error());
        return (this->_error_code);
    }
    if (slot == EQUIP_HEAD)
        this->_head = item;
    else if (slot == EQUIP_CHEST)
        this->_chest = item;
    else if (slot == EQUIP_WEAPON)
        this->_weapon = item;
    else
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error_code);
    }
    return (ER_SUCCESS);
}

void ft_equipment::unequip(int slot) noexcept
{
    if (slot == EQUIP_HEAD)
        this->_head = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_CHEST)
        this->_chest = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_WEAPON)
        this->_weapon = ft_sharedptr<ft_item>();
    else
        this->set_error(GAME_GENERAL_ERROR);
    return ;
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) noexcept
{
    if (slot == EQUIP_HEAD)
        return (this->_head);
    else if (slot == EQUIP_CHEST)
        return (this->_chest);
    else if (slot == EQUIP_WEAPON)
        return (this->_weapon);
    this->set_error(GAME_GENERAL_ERROR);
    return (ft_sharedptr<ft_item>());
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) const noexcept
{
    if (slot == EQUIP_HEAD)
        return (this->_head);
    else if (slot == EQUIP_CHEST)
        return (this->_chest);
    else if (slot == EQUIP_WEAPON)
        return (this->_weapon);
    this->set_error(GAME_GENERAL_ERROR);
    return (ft_sharedptr<ft_item>());
}

int ft_equipment::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_equipment::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
