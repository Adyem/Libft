#include "../PThread/pthread_internal.hpp"
#include "game_equipment.hpp"
#include <new>

thread_local int ft_equipment::_last_error = FT_ERR_SUCCESS;

void ft_equipment::set_error(int error_code) const noexcept
{
    ft_equipment::_last_error = error_code;
    return ;
}

ft_equipment::ft_equipment() noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
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
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_equipment::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_equipment::equip(int slot, const ft_sharedptr<ft_item> &item) noexcept
{
    bool lock_acquired;
    int lock_error;
    int result;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->validate_item(item) == false)
    {
        this->unlock_internal(lock_acquired);
        result = FT_ERR_INVALID_ARGUMENT;
        this->set_error(result);
        return (result);
    }
    if (slot == EQUIP_HEAD)
    {
        this->_head = item;
        result = FT_ERR_SUCCESS;
    }
    else if (slot == EQUIP_CHEST)
    {
        this->_chest = item;
        result = FT_ERR_SUCCESS;
    }
    else if (slot == EQUIP_WEAPON)
    {
        this->_weapon = item;
        result = FT_ERR_SUCCESS;
    }
    else
    {
        result = FT_ERR_INVALID_ARGUMENT;
    }
    this->unlock_internal(lock_acquired);
    this->set_error(result);
    return (result);
}

void ft_equipment::unequip(int slot) noexcept
{
    bool lock_acquired;
    int lock_error;
    int result;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (slot == EQUIP_HEAD)
    {
        this->_head = ft_sharedptr<ft_item>();
        result = FT_ERR_SUCCESS;
    }
    else if (slot == EQUIP_CHEST)
    {
        this->_chest = ft_sharedptr<ft_item>();
        result = FT_ERR_SUCCESS;
    }
    else if (slot == EQUIP_WEAPON)
    {
        this->_weapon = ft_sharedptr<ft_item>();
        result = FT_ERR_SUCCESS;
    }
    else
    {
        result = FT_ERR_INVALID_ARGUMENT;
    }
    this->unlock_internal(lock_acquired);
    this->set_error(result);
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
    {
        this->set_error(lock_error);
        return (result);
    }
    if (slot == EQUIP_HEAD)
    {
        result = this->_head;
        this->set_error(FT_ERR_SUCCESS);
    }
    else if (slot == EQUIP_CHEST)
    {
        result = this->_chest;
        this->set_error(FT_ERR_SUCCESS);
    }
    else if (slot == EQUIP_WEAPON)
    {
        result = this->_weapon;
        this->set_error(FT_ERR_SUCCESS);
    }
    else
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_sharedptr<ft_item>());
    }
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
    {
        this->set_error(lock_error);
        return (result);
    }
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    else
    {
        this->unlock_internal(lock_acquired);
        const_cast<ft_equipment *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_sharedptr<ft_item>());
    }
    this->unlock_internal(lock_acquired);
    const_cast<ft_equipment *>(this)->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_equipment::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_equipment::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_equipment::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_equipment::get_error() const noexcept
{
    return (ft_equipment::_last_error);
}

const char *ft_equipment::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
