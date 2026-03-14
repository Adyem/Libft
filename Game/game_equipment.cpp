#include "../PThread/pthread_internal.hpp"
#include "game_equipment.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t ft_equipment::_last_error = FT_ERR_SUCCESS;

int32_t ft_equipment::set_error(int32_t error_code) noexcept
{
    ft_equipment::_last_error = error_code;
    return (error_code);
}

ft_equipment::ft_equipment() noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_equipment::ft_equipment(const ft_equipment &other) noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;
    int32_t equip_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_equipment::ft_equipment(copy)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return ;
    }
    this->_head = other._head;
    this->_chest = other._chest;
    this->_weapon = other._weapon;
    equip_error = this->get_error();
    if (equip_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(equip_error);
        return ;
    }
    this->set_error(other.get_error());
    return ;
}

ft_equipment::ft_equipment(ft_equipment &&other) noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_equipment::ft_equipment(move)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

ft_equipment::~ft_equipment() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t ft_equipment::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_equipment::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_head = ft_sharedptr<ft_item>();
    this->_chest = ft_sharedptr<ft_item>();
    this->_weapon = ft_sharedptr<ft_item>();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_equipment::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_head = ft_sharedptr<ft_item>();
    this->_chest = ft_sharedptr<ft_item>();
    this->_weapon = ft_sharedptr<ft_item>();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_equipment::move(ft_equipment &other) noexcept
{
    int32_t destroy_error;
    int32_t initialize_error;
    int32_t source_error;
    int32_t source_destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_equipment::move",
            "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_head = other._head;
    this->_chest = other._chest;
    this->_weapon = other._weapon;
    source_error = other.get_error();
    source_destroy_error = other.destroy();
    if (source_destroy_error != FT_ERR_SUCCESS)
    {
        this->set_error(source_destroy_error);
        return (source_destroy_error);
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

ft_bool ft_equipment::validate_item(const ft_sharedptr<ft_item> &item) const noexcept
{
    if (!item)
        return (FT_FALSE);
    return (FT_TRUE);
}

int32_t ft_equipment::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_equipment::unlock_internal(ft_bool lock_acquired) const noexcept
{

    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_equipment::equip(int32_t slot, const ft_sharedptr<ft_item> &item) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::equip");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->validate_item(item) == FT_FALSE)
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

void ft_equipment::unequip(int32_t slot) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::unequip");
    lock_acquired = FT_FALSE;
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

ft_sharedptr<ft_item> ft_equipment::get_item(int32_t slot) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_sharedptr<ft_item> result;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::get_item");
    result = ft_sharedptr<ft_item>();
    lock_acquired = FT_FALSE;
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

ft_sharedptr<ft_item> ft_equipment::get_item(int32_t slot) const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_sharedptr<ft_item> result;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::get_item const");
    result = ft_sharedptr<ft_item>();
    lock_acquired = FT_FALSE;
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
    const_cast<ft_equipment *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

int32_t ft_equipment::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::enable_thread_safety");
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

int32_t ft_equipment::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int32_t destroy_error;

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

ft_bool ft_equipment::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_equipment::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::get_error");
    return (ft_equipment::_last_error);
}

const char *ft_equipment::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state, "ft_equipment::get_error_str");
    return (ft_strerror(this->get_error()));
}
