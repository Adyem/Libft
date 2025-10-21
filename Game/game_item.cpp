#include "game_item.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

static void game_item_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_item_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_item::lock_pair(const ft_item &first, const ft_item &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_item *ordered_first;
    const ft_item *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_item *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_item_sleep_backoff();
    }
}

ft_item::ft_item() noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1{0, 0}, _modifier2{0, 0},
      _modifier3{0, 0}, _modifier4{0, 0}, _error_code(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_item::ft_item(const ft_item &other) noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1{0, 0}, _modifier2{0, 0},
      _modifier3{0, 0}, _modifier4{0, 0}, _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_item_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_item_restore_errno(other_guard, entry_errno);
    return ;
}

ft_item &ft_item::operator=(const ft_item &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_item::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_item_restore_errno(this_guard, entry_errno);
    game_item_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_item::ft_item(ft_item &&other) noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1{0, 0}, _modifier2{0, 0},
      _modifier3{0, 0}, _modifier4{0, 0}, _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_item_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error_code = other._error_code;
    other._max_stack = 0;
    other._stack_size = 0;
    other._item_id = 0;
    other._rarity = 0;
    other._width = 1;
    other._height = 1;
    other._modifier1.id = 0;
    other._modifier1.value = 0;
    other._modifier2.id = 0;
    other._modifier2.value = 0;
    other._modifier3.id = 0;
    other._modifier3.value = 0;
    other._modifier4.id = 0;
    other._modifier4.value = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_item_restore_errno(other_guard, entry_errno);
    return ;
}

ft_item &ft_item::operator=(ft_item &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_item::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error_code = other._error_code;
    other._max_stack = 0;
    other._stack_size = 0;
    other._item_id = 0;
    other._rarity = 0;
    other._width = 1;
    other._height = 1;
    other._modifier1.id = 0;
    other._modifier1.value = 0;
    other._modifier2.id = 0;
    other._modifier2.value = 0;
    other._modifier3.id = 0;
    other._modifier3.value = 0;
    other._modifier4.id = 0;
    other._modifier4.value = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_item_restore_errno(this_guard, entry_errno);
    game_item_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_item::get_max_stack() const noexcept
{
    int entry_errno;
    int max_stack_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    max_stack_value = this->_max_stack;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (max_stack_value);
}

void ft_item::set_max_stack(int max) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_max_stack = max;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_stack_size() const noexcept
{
    int entry_errno;
    int stack_size_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    stack_size_value = this->_stack_size;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (stack_size_value);
}

void ft_item::set_stack_size(int amount) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_stack_size = amount;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

void ft_item::add_to_stack(int amount) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_stack_size += amount;
    if (this->_stack_size > this->_max_stack)
        this->_stack_size = this->_max_stack;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

void ft_item::sub_from_stack(int amount) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_stack_size -= amount;
    if (this->_stack_size < 0)
        this->_stack_size = 0;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_item_id() const noexcept
{
    int entry_errno;
    int item_identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    item_identifier = this->_item_id;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (item_identifier);
}

void ft_item::set_item_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_item_id = id;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_width() const noexcept
{
    int entry_errno;
    int width_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    width_value = this->_width;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (width_value);
}

void ft_item::set_width(int width) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_width = width;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_height() const noexcept
{
    int entry_errno;
    int height_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    height_value = this->_height;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (height_value);
}

void ft_item::set_height(int height) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_height = height;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_rarity() const noexcept
{
    int entry_errno;
    int rarity_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    rarity_value = this->_rarity;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (rarity_value);
}

void ft_item::set_rarity(int rarity) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

ft_item_modifier ft_item::get_modifier1() const noexcept
{
    int entry_errno;
    ft_item_modifier modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (ft_item_modifier{0, 0});
    }
    modifier = this->_modifier1;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_item::set_modifier1(const ft_item_modifier &mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier1_id() const noexcept
{
    int entry_errno;
    int modifier_identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_identifier = this->_modifier1.id;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_identifier);
}

void ft_item::set_modifier1_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1.id = id;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier1_value() const noexcept
{
    int entry_errno;
    int modifier_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_value = this->_modifier1.value;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_value);
}

void ft_item::set_modifier1_value(int value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1.value = value;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

ft_item_modifier ft_item::get_modifier2() const noexcept
{
    int entry_errno;
    ft_item_modifier modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (ft_item_modifier{0, 0});
    }
    modifier = this->_modifier2;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_item::set_modifier2(const ft_item_modifier &mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier2_id() const noexcept
{
    int entry_errno;
    int modifier_identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_identifier = this->_modifier2.id;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_identifier);
}

void ft_item::set_modifier2_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2.id = id;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier2_value() const noexcept
{
    int entry_errno;
    int modifier_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_value = this->_modifier2.value;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_value);
}

void ft_item::set_modifier2_value(int value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2.value = value;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

ft_item_modifier ft_item::get_modifier3() const noexcept
{
    int entry_errno;
    ft_item_modifier modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (ft_item_modifier{0, 0});
    }
    modifier = this->_modifier3;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_item::set_modifier3(const ft_item_modifier &mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier3_id() const noexcept
{
    int entry_errno;
    int modifier_identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_identifier = this->_modifier3.id;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_identifier);
}

void ft_item::set_modifier3_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3.id = id;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier3_value() const noexcept
{
    int entry_errno;
    int modifier_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_value = this->_modifier3.value;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_value);
}

void ft_item::set_modifier3_value(int value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3.value = value;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

ft_item_modifier ft_item::get_modifier4() const noexcept
{
    int entry_errno;
    ft_item_modifier modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (ft_item_modifier{0, 0});
    }
    modifier = this->_modifier4;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_item::set_modifier4(const ft_item_modifier &mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier4_id() const noexcept
{
    int entry_errno;
    int modifier_identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_identifier = this->_modifier4.id;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_identifier);
}

void ft_item::set_modifier4_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4.id = id;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

int ft_item::get_modifier4_value() const noexcept
{
    int entry_errno;
    int modifier_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier_value = this->_modifier4.value;
    const_cast<ft_item *>(this)->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return (modifier_value);
}

void ft_item::set_modifier4_value(int value) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4.value = value;
    this->set_error(ER_SUCCESS);
    game_item_restore_errno(guard, entry_errno);
    return ;
}

void ft_item::set_error(int err) const noexcept
{
    this->_error_code = err;
    ft_errno = err;
    return ;
}

int ft_item::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_item *>(this)->set_error(error_code);
    game_item_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_item::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.get_error());
        game_item_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_item *>(this)->set_error(error_code);
    game_item_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}
