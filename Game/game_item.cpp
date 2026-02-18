#include "game_item.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

static void game_item_modifier_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static int game_item_reset_modifier(ft_item_modifier &modifier)
{
    modifier.set_id(0);
    modifier.set_value(0);
    return (FT_ERR_SUCCESS);
}

ft_item_modifier::ft_item_modifier() noexcept
    : _id(0), _value(0), _mutex()
{
    return ;
}

ft_item_modifier::ft_item_modifier(int id, int value) noexcept
    : _id(id), _value(value), _mutex()
{
    return ;
}

ft_item_modifier::ft_item_modifier(const ft_item_modifier &other) noexcept
    : _id(0), _value(0), _mutex()
{
    int lock_error;
    int unlock_error;

    lock_error = other._mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_id = other._id;
    this->_value = other._value;
    unlock_error = other._mutex.unlock();
    (void)unlock_error;
    return ;
}

ft_item_modifier &ft_item_modifier::operator=(const ft_item_modifier &other) noexcept
{
    int lock_error;
    int unlock_error;
    const ft_item_modifier *lower;
    const ft_item_modifier *upper;
    bool swapped;

    if (this == &other)
        return (*this);
    lower = this;
    upper = &other;
    swapped = false;
    if (lower > upper)
    {
        lower = &other;
        upper = this;
        swapped = true;
    }
    lock_error = lower->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (*this);
    while (true)
    {
        lock_error = upper->_mutex.lock();
        if (lock_error == FT_ERR_SUCCESS)
            break ;
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            unlock_error = lower->_mutex.unlock();
            (void)unlock_error;
            return (*this);
        }
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
        game_item_modifier_sleep_backoff();
        lock_error = lower->_mutex.lock();
        if (lock_error != FT_ERR_SUCCESS)
            return (*this);
    }
    this->_id = other._id;
    this->_value = other._value;
    if (!swapped)
    {
        unlock_error = upper->_mutex.unlock();
        (void)unlock_error;
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
    }
    else
    {
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
        unlock_error = upper->_mutex.unlock();
        (void)unlock_error;
    }
    return (*this);
}

ft_item_modifier::ft_item_modifier(ft_item_modifier &&other) noexcept
    : _id(0), _value(0), _mutex()
{
    int lock_error;
    int unlock_error;

    lock_error = other._mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_id = other._id;
    this->_value = other._value;
    other._id = 0;
    other._value = 0;
    unlock_error = other._mutex.unlock();
    (void)unlock_error;
    return ;
}

ft_item_modifier &ft_item_modifier::operator=(ft_item_modifier &&other) noexcept
{
    int lock_error;
    int unlock_error;
    const ft_item_modifier *lower;
    const ft_item_modifier *upper;
    bool swapped;

    if (this == &other)
        return (*this);
    lower = this;
    upper = &other;
    swapped = false;
    if (lower > upper)
    {
        lower = &other;
        upper = this;
        swapped = true;
    }
    lock_error = lower->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (*this);
    while (true)
    {
        lock_error = upper->_mutex.lock();
        if (lock_error == FT_ERR_SUCCESS)
            break ;
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            unlock_error = lower->_mutex.unlock();
            (void)unlock_error;
            return (*this);
        }
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
        game_item_modifier_sleep_backoff();
        lock_error = lower->_mutex.lock();
        if (lock_error != FT_ERR_SUCCESS)
            return (*this);
    }
    this->_id = other._id;
    this->_value = other._value;
    other._id = 0;
    other._value = 0;
    if (!swapped)
    {
        unlock_error = upper->_mutex.unlock();
        (void)unlock_error;
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
    }
    else
    {
        unlock_error = lower->_mutex.unlock();
        (void)unlock_error;
        unlock_error = upper->_mutex.unlock();
        (void)unlock_error;
    }
    return (*this);
}

int ft_item_modifier::get_id() const noexcept
{
    int modifier_id;
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    modifier_id = this->_id;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return (modifier_id);
}

void ft_item_modifier::set_id(int id) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_id = id;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return ;
}

int ft_item_modifier::get_value() const noexcept
{
    int modifier_value;
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    modifier_value = this->_value;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return (modifier_value);
}

void ft_item_modifier::set_value(int value) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_value = value;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return ;
}

int ft_item_modifier::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_item_modifier::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}

static void game_item_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_item_unlock(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
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

        if (single_guard.last_operation_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.last_operation_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
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

        if (lower_guard.last_operation_error() != FT_ERR_SUCCESS)
        {
            return (lower_guard.last_operation_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.last_operation_error() == FT_ERR_SUCCESS)
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
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.last_operation_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.last_operation_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_item_sleep_backoff();
    }
}

ft_item::ft_item() noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1(), _modifier2(),
      _modifier3(), _modifier4(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_item::ft_item(const ft_item &other) noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1(), _modifier2(),
      _modifier3(), _modifier4(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.last_operation_error());
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
    game_item_unlock(other_guard);
    return ;
}

ft_item &ft_item::operator=(const ft_item &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_item::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
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
    game_item_unlock(this_guard);
    game_item_unlock(other_guard);
    return (*this);
}

ft_item::ft_item(ft_item &&other) noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1(), _modifier2(),
      _modifier3(), _modifier4(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    int modifier_error;

    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.last_operation_error());
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
    modifier_error = game_item_reset_modifier(other._modifier1);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    modifier_error = game_item_reset_modifier(other._modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    modifier_error = game_item_reset_modifier(other._modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    modifier_error = game_item_reset_modifier(other._modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    game_item_unlock(other_guard);
    return ;
}

ft_item &ft_item::operator=(ft_item &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_item::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
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
    int modifier_error;

    modifier_error = game_item_reset_modifier(other._modifier1);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        game_item_unlock(this_guard);
        game_item_unlock(other_guard);
        return (*this);
    }
    modifier_error = game_item_reset_modifier(other._modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        game_item_unlock(this_guard);
        game_item_unlock(other_guard);
        return (*this);
    }
    modifier_error = game_item_reset_modifier(other._modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        game_item_unlock(this_guard);
        game_item_unlock(other_guard);
        return (*this);
    }
    modifier_error = game_item_reset_modifier(other._modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        game_item_unlock(this_guard);
        game_item_unlock(other_guard);
        return (*this);
    }
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    game_item_unlock(this_guard);
    game_item_unlock(other_guard);
    return (*this);
}

int ft_item::get_max_stack() const noexcept
{
    int max_stack_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    max_stack_value = this->_max_stack;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (max_stack_value);
}

void ft_item::set_max_stack(int max) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_max_stack = max;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_stack_size() const noexcept
{
    int stack_size_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    stack_size_value = this->_stack_size;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (stack_size_value);
}

void ft_item::set_stack_size(int amount) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_stack_size = amount;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

void ft_item::add_to_stack(int amount) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_stack_size += amount;
    if (this->_stack_size > this->_max_stack)
        this->_stack_size = this->_max_stack;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

void ft_item::sub_from_stack(int amount) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_stack_size -= amount;
    if (this->_stack_size < 0)
        this->_stack_size = 0;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_item_id() const noexcept
{
    int item_identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    item_identifier = this->_item_id;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (item_identifier);
}

void ft_item::set_item_id(int id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_item_id = id;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_width() const noexcept
{
    int width_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    width_value = this->_width;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (width_value);
}

void ft_item::set_width(int width) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_width = width;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_height() const noexcept
{
    int height_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    height_value = this->_height;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (height_value);
}

void ft_item::set_height(int height) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_height = height;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_rarity() const noexcept
{
    int rarity_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    rarity_value = this->_rarity;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (rarity_value);
}

void ft_item::set_rarity(int rarity) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

ft_item_modifier ft_item::get_modifier1() const noexcept
{
    ft_item_modifier modifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (ft_item_modifier());
    }
    modifier = this->_modifier1;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (ft_item_modifier());
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier);
}

void ft_item::set_modifier1(const ft_item_modifier &mod) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->_modifier1 = mod;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier1_id() const noexcept
{
    int modifier_identifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_identifier = this->_modifier1.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_identifier);
}

void ft_item::set_modifier1_id(int id) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier1.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier1_value() const noexcept
{
    int modifier_value;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_value = this->_modifier1.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_value);
}

void ft_item::set_modifier1_value(int value) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier1.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_item_modifier ft_item::get_modifier2() const noexcept
{
    ft_item_modifier modifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (ft_item_modifier());
    }
    modifier = this->_modifier2;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (ft_item_modifier());
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier);
}

void ft_item::set_modifier2(const ft_item_modifier &mod) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->_modifier2 = mod;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier2_id() const noexcept
{
    int modifier_identifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_identifier = this->_modifier2.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_identifier);
}

void ft_item::set_modifier2_id(int id) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier2.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier2_value() const noexcept
{
    int modifier_value;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_value = this->_modifier2.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_value);
}

void ft_item::set_modifier2_value(int value) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier2.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_item_modifier ft_item::get_modifier3() const noexcept
{
    ft_item_modifier modifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (ft_item_modifier());
    }
    modifier = this->_modifier3;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (ft_item_modifier());
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier);
}

void ft_item::set_modifier3(const ft_item_modifier &mod) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->_modifier3 = mod;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier3_id() const noexcept
{
    int modifier_identifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_identifier = this->_modifier3.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_identifier);
}

void ft_item::set_modifier3_id(int id) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier3.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item::get_modifier3_value() const noexcept
{
    int modifier_value;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_value = this->_modifier3.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    return (modifier_value);
}

void ft_item::set_modifier3_value(int value) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier3.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_item_modifier ft_item::get_modifier4() const noexcept
{
    ft_item_modifier modifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (ft_item_modifier());
    }
    modifier = this->_modifier4;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (ft_item_modifier());
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (modifier);
}

void ft_item::set_modifier4(const ft_item_modifier &mod) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->_modifier4 = mod;
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_modifier4_id() const noexcept
{
    int modifier_identifier;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_identifier = this->_modifier4.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (modifier_identifier);
}

void ft_item::set_modifier4_id(int id) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier4.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

int ft_item::get_modifier4_value() const noexcept
{
    int modifier_value;
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (0);
    }
    modifier_value = this->_modifier4.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return (modifier_value);
}

void ft_item::set_modifier4_value(int value) noexcept
{
    int modifier_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.last_operation_error());
        return ;
    }
    this->_modifier4.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    game_item_unlock(guard);
    return ;
}

void ft_item::set_error(int err) const noexcept
{
    this->_error_code = err;
    return ;
}

int ft_item::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (guard.last_operation_error());
    }
    error_code = this->_error_code;
    const_cast<ft_item *>(this)->set_error(error_code);
    game_item_unlock(guard);
    return (error_code);
}

const char *ft_item::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.last_operation_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(guard.last_operation_error());
        return (ft_strerror(guard.last_operation_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_item *>(this)->set_error(error_code);
    game_item_unlock(guard);
    return (ft_strerror(error_code));
}
