#include "game_equipment.hpp"

ft_equipment::ft_equipment() noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex()
{
    return ;
}

bool ft_equipment::validate_item(const ft_sharedptr<ft_item> &item) const noexcept
{
    if (!item)
        return (false);
    return (true);
}

int ft_equipment::equip(int slot, const ft_sharedptr<ft_item> &item) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->validate_item(item) == false)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (slot == EQUIP_HEAD)
        this->_head = item;
    else if (slot == EQUIP_CHEST)
        this->_chest = item;
    else if (slot == EQUIP_WEAPON)
        this->_weapon = item;
    else
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return (FT_ERR_SUCCESS);
}

void ft_equipment::unequip(int slot) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (slot == EQUIP_HEAD)
        this->_head = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_CHEST)
        this->_chest = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_WEAPON)
        this->_weapon = ft_sharedptr<ft_item>();
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return ;
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) noexcept
{
    int lock_error;
    int unlock_error;
    ft_sharedptr<ft_item> result;

    result = ft_sharedptr<ft_item>();
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return (result);
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) const noexcept
{
    int lock_error;
    int unlock_error;
    ft_sharedptr<ft_item> result;

    result = ft_sharedptr<ft_item>();
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return (result);
}

int ft_equipment::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_equipment::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}
