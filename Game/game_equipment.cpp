#include "game_equipment.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_equipment_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_equipment_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    (void)entry_errno;
    return ;
}

int ft_equipment::lock_pair(const ft_equipment &first, const ft_equipment &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_equipment *ordered_first;
    const ft_equipment *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_equipment *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_equipment_sleep_backoff();
    }
}

ft_equipment::ft_equipment() noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_equipment::ft_equipment(const ft_equipment &other) noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->_head = ft_sharedptr<ft_item>();
        this->_chest = ft_sharedptr<ft_item>();
        this->_weapon = ft_sharedptr<ft_item>();
        this->set_error(other_guard.get_error());
        game_equipment_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_head = other._head;
    this->_chest = other._chest;
    this->_weapon = other._weapon;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_equipment_restore_errno(other_guard, entry_errno);
    return ;
}

ft_equipment &ft_equipment::operator=(const ft_equipment &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_equipment::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_head = other._head;
    this->_chest = other._chest;
    this->_weapon = other._weapon;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_equipment_restore_errno(this_guard, entry_errno);
    game_equipment_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_equipment::ft_equipment(ft_equipment &&other) noexcept
    : _head(ft_sharedptr<ft_item>()), _chest(ft_sharedptr<ft_item>()),
      _weapon(ft_sharedptr<ft_item>()), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->_head = ft_sharedptr<ft_item>();
        this->_chest = ft_sharedptr<ft_item>();
        this->_weapon = ft_sharedptr<ft_item>();
        this->set_error(other_guard.get_error());
        game_equipment_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_head = ft_move(other._head);
    this->_chest = ft_move(other._chest);
    this->_weapon = ft_move(other._weapon);
    this->_error_code = other._error_code;
    other._head = ft_sharedptr<ft_item>();
    other._chest = ft_sharedptr<ft_item>();
    other._weapon = ft_sharedptr<ft_item>();
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    game_equipment_restore_errno(other_guard, entry_errno);
    return ;
}

ft_equipment &ft_equipment::operator=(ft_equipment &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_equipment::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_head = ft_move(other._head);
    this->_chest = ft_move(other._chest);
    this->_weapon = ft_move(other._weapon);
    this->_error_code = other._error_code;
    other._head = ft_sharedptr<ft_item>();
    other._chest = ft_sharedptr<ft_item>();
    other._weapon = ft_sharedptr<ft_item>();
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    game_equipment_restore_errno(this_guard, entry_errno);
    game_equipment_restore_errno(other_guard, entry_errno);
    return (*this);
}

void ft_equipment::set_error(int err) const noexcept
{
    this->_error_code = err;
    ft_errno = err;
    return ;
}

bool ft_equipment::validate_item(const ft_sharedptr<ft_item> &item) noexcept
{
    if (!item)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    if (item.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(item.get_error());
        return (true);
    }
    if (item->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(item->get_error());
        return (true);
    }
    return (false);
}

int ft_equipment::equip(int slot, const ft_sharedptr<ft_item> &item) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(FT_ERR_SUCCESSS);
    if (this->validate_item(item) == true)
    {
        game_equipment_restore_errno(guard, entry_errno);
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
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_equipment_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->set_error(FT_ERR_SUCCESSS);
    game_equipment_restore_errno(guard, entry_errno);
    return (FT_ERR_SUCCESSS);
}

void ft_equipment::unequip(int slot) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    if (slot == EQUIP_HEAD)
        this->_head = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_CHEST)
        this->_chest = ft_sharedptr<ft_item>();
    else if (slot == EQUIP_WEAPON)
        this->_weapon = ft_sharedptr<ft_item>();
    else
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    game_equipment_restore_errno(guard, entry_errno);
    return ;
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) noexcept
{
    int entry_errno;
    ft_sharedptr<ft_item> result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    this->set_error(FT_ERR_SUCCESSS);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    else
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        result = ft_sharedptr<ft_item>();
    }
    game_equipment_restore_errno(guard, entry_errno);
    return (result);
}

ft_sharedptr<ft_item> ft_equipment::get_item(int slot) const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_item> result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_equipment *>(this)->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    const_cast<ft_equipment *>(this)->set_error(FT_ERR_SUCCESSS);
    if (slot == EQUIP_HEAD)
        result = this->_head;
    else if (slot == EQUIP_CHEST)
        result = this->_chest;
    else if (slot == EQUIP_WEAPON)
        result = this->_weapon;
    else
    {
        const_cast<ft_equipment *>(this)->set_error(FT_ERR_GAME_GENERAL_ERROR);
        result = ft_sharedptr<ft_item>();
    }
    game_equipment_restore_errno(guard, entry_errno);
    return (result);
}

int ft_equipment::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_equipment *>(this)->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_equipment *>(this)->set_error(error_code);
    game_equipment_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_equipment::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_equipment *>(this)->set_error(guard.get_error());
        game_equipment_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_equipment *>(this)->set_error(error_code);
    game_equipment_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}
