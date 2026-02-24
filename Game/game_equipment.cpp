#include "game_equipment.hpp"
#include <new>

ft_equipment::ft_equipment() noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex(ft_nullptr)
{
    return ;
}

ft_equipment::~ft_equipment() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

bool ft_equipment::validate_item(const ft_sharedptr<ft_item> &item) const noexcept
{
    if (!item)
        return (false);
    return (true);
}

int ft_equipment::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_equipment::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

int ft_equipment::equip(int slot, const ft_sharedptr<ft_item> &item) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->validate_item(item) == false)
    {
        this->unlock_internal(lock_acquired);
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
        this->unlock_internal(lock_acquired);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_equipment::unequip(int slot) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (slot == EQUIP_HEAD)
        this->_head = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_CHEST)
        this->_chest = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_WEAPON)
        this->_weapon = ft_sharedptr<ft_item>();
    this->unlock_internal(lock_acquired);
    return ;
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_sharedptr<ft_item> result;

    result = ft_sharedptr<ft_item>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    this->unlock_internal(lock_acquired);
    return (result);
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) const noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_sharedptr<ft_item> result;

    result = ft_sharedptr<ft_item>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    this->unlock_internal(lock_acquired);
    return (result);
}

int ft_equipment::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_equipment::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_equipment::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_equipment::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_equipment::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}
