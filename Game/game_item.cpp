#include "../PThread/pthread_internal.hpp"
#include "game_item.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread.hpp"
#include <new>

thread_local int ft_item_modifier::_last_error = FT_ERR_SUCCESS;
thread_local int ft_item::_last_error = FT_ERR_SUCCESS;
static int game_item_reset_modifier(ft_item_modifier &modifier)
{
    modifier.set_id(0);
    modifier.set_value(0);
    return (FT_ERR_SUCCESS);
}

ft_item_modifier::ft_item_modifier() noexcept
    : _id(0), _value(0), _mutex(ft_nullptr)
{
    return ;
}

ft_item_modifier::~ft_item_modifier() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

void ft_item_modifier::set_error(int error_code) const noexcept
{
    ft_item_modifier::_last_error = error_code;
    return ;
}

int ft_item_modifier::initialize() noexcept
{
    this->_id = 0;
    this->_value = 0;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_modifier::initialize(int id, int value) noexcept
{
    this->_id = id;
    this->_value = value;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_modifier::initialize(const ft_item_modifier &other) noexcept
{
    int lock_error;
    int unlock_error;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_id = other._id;
    this->_value = other._value;
    if (other._mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_modifier::initialize(ft_item_modifier &&other) noexcept
{
    int lock_error;
    int unlock_error;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_id = other._id;
    this->_value = other._value;
    other._id = 0;
    other._value = 0;
    if (other._mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(other._mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_modifier::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item_modifier::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

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

bool ft_item_modifier::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_item_modifier::get_id() const noexcept
{
    int modifier_id;
    int lock_error;
    int unlock_error;

    if (this->_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    modifier_id = this->_id;
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return (modifier_id);
}

void ft_item_modifier::set_id(int id) noexcept
{
    int lock_error;
    int unlock_error;

    if (this->_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_id = id;
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item_modifier::get_value() const noexcept
{
    int modifier_value;
    int lock_error;
    int unlock_error;

    if (this->_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    modifier_value = this->_value;
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return (modifier_value);
}

void ft_item_modifier::set_value(int value) noexcept
{
    int lock_error;
    int unlock_error;

    if (this->_mutex == ft_nullptr)
        lock_error = FT_ERR_SUCCESS;
    else
        lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_value = value;
    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    (void)unlock_error;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_item_modifier::get_error() const noexcept
{
    return (ft_item_modifier::_last_error);
}

const char *ft_item_modifier::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

static void game_item_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int ft_item::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_item::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int ft_item::lock_pair(const ft_item &first, const ft_item &second,
        bool *first_locked, bool *second_locked)
{
    const ft_item *ordered_first;
    const ft_item *ordered_second;
    int lock_error;
    bool swapped;

    if (first_locked != ft_nullptr)
        *first_locked = false;
    if (second_locked != ft_nullptr)
        *second_locked = false;
    if (&first == &second)
        return (first.lock_internal(first_locked));
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
        bool lower_locked;
        bool upper_locked;

        lower_locked = false;
        upper_locked = false;
        lock_error = ordered_first->lock_internal(&lower_locked);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
        lock_error = ordered_second->lock_internal(&upper_locked);
        if (lock_error == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                if (first_locked != ft_nullptr)
                    *first_locked = lower_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = upper_locked;
            }
            else
            {
                if (first_locked != ft_nullptr)
                    *first_locked = upper_locked;
                if (second_locked != ft_nullptr)
                    *second_locked = lower_locked;
            }
            return (FT_ERR_SUCCESS);
        }
        if (lock_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_internal(lower_locked);
            return (lock_error);
        }
        ordered_first->unlock_internal(lower_locked);
        game_item_sleep_backoff();
    }
}

ft_item::ft_item() noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1(), _modifier2(),
      _modifier3(), _modifier4(), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_item::~ft_item() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_item::initialize() noexcept
{
    this->_max_stack = 0;
    this->_stack_size = 0;
    this->_item_id = 0;
    this->_rarity = 0;
    this->_width = 1;
    this->_height = 1;
    (void)this->_modifier1.initialize();
    (void)this->_modifier2.initialize();
    (void)this->_modifier3.initialize();
    (void)this->_modifier4.initialize();
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_item::initialize(const ft_item &other) noexcept
{
    bool other_locked;
    int lock_error;
    int modifier_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    other_locked = false;
    lock_error = other.lock_internal(&other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    modifier_error = this->_modifier1.initialize(other._modifier1);
    if (modifier_error != FT_ERR_SUCCESS)
        return (modifier_error);
    modifier_error = this->_modifier2.initialize(other._modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
        return (modifier_error);
    modifier_error = this->_modifier3.initialize(other._modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
        return (modifier_error);
    modifier_error = this->_modifier4.initialize(other._modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
        return (modifier_error);
    this->set_error(other.get_error());
    other.unlock_internal(other_locked);
    return (FT_ERR_SUCCESS);
}

int ft_item::initialize(ft_item &&other) noexcept
{
    bool this_locked;
    bool other_locked;
    int lock_error;
    int modifier_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    this_locked = false;
    other_locked = false;
    lock_error = ft_item::lock_pair(*this, other, &this_locked, &other_locked);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_max_stack = other._max_stack;
    this->_stack_size = other._stack_size;
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_width = other._width;
    this->_height = other._height;
    modifier_error = this->_modifier1.initialize(other._modifier1);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = this->_modifier2.initialize(other._modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = this->_modifier3.initialize(other._modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = this->_modifier4.initialize(other._modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    this->set_error(other.get_error());
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
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = game_item_reset_modifier(other._modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = game_item_reset_modifier(other._modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    modifier_error = game_item_reset_modifier(other._modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(this_locked);
        other.unlock_internal(other_locked);
        return (modifier_error);
    }
    other.set_error(FT_ERR_SUCCESS);
    this->unlock_internal(this_locked);
    other.unlock_internal(other_locked);
    return (FT_ERR_SUCCESS);
}

int ft_item::enable_thread_safety() noexcept
{
    int initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    return (FT_ERR_SUCCESS);
}

int ft_item::disable_thread_safety() noexcept
{
    int destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    return (destroy_error);
}

bool ft_item::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_item::get_max_stack() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int max_stack_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    max_stack_value = this->_max_stack;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (max_stack_value);
}

void ft_item::set_max_stack(int max) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_max_stack = max;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_stack_size() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int stack_size_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    stack_size_value = this->_stack_size;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (stack_size_value);
}

void ft_item::set_stack_size(int amount) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_stack_size = amount;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_item::add_to_stack(int amount) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_stack_size += amount;
    if (this->_stack_size > this->_max_stack)
        this->_stack_size = this->_max_stack;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_item::sub_from_stack(int amount) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_stack_size -= amount;
    if (this->_stack_size < 0)
        this->_stack_size = 0;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_item_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int item_identifier;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    item_identifier = this->_item_id;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (item_identifier);
}

void ft_item::set_item_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_item_id = id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_width() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int width_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    width_value = this->_width;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (width_value);
}

void ft_item::set_width(int width) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_width = width;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_height() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int height_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    height_value = this->_height;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (height_value);
}

void ft_item::set_height(int height) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_height = height;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_rarity() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int rarity_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    rarity_value = this->_rarity;
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (rarity_value);
}

void ft_item::set_rarity(int rarity) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier1(ft_item_modifier &modifier) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (lock_error);
    }
    modifier_error = modifier.initialize(this->_modifier1);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (modifier_error);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_item::set_modifier1(const ft_item_modifier &mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    modifier_error = this->_modifier1.initialize(mod);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier1_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_identifier;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_identifier = this->_modifier1.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_identifier);
}

void ft_item::set_modifier1_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier1.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier1_value() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_value;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_value = this->_modifier1.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_value);
}

void ft_item::set_modifier1_value(int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier1.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier2(ft_item_modifier &modifier) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (lock_error);
    }
    modifier_error = modifier.initialize(this->_modifier2);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (modifier_error);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_item::set_modifier2(const ft_item_modifier &mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    modifier_error = this->_modifier2.initialize(mod);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier2_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_identifier;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_identifier = this->_modifier2.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_identifier);
}

void ft_item::set_modifier2_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier2.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier2_value() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_value;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_value = this->_modifier2.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_value);
}

void ft_item::set_modifier2_value(int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier2.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier3(ft_item_modifier &modifier) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (lock_error);
    }
    modifier_error = modifier.initialize(this->_modifier3);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (modifier_error);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_item::set_modifier3(const ft_item_modifier &mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    modifier_error = this->_modifier3.initialize(mod);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier3_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_identifier;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_identifier = this->_modifier3.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_identifier);
}

void ft_item::set_modifier3_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier3.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier3_value() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_value;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_value = this->_modifier3.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_value);
}

void ft_item::set_modifier3_value(int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier3.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier4(ft_item_modifier &modifier) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (lock_error);
    }
    modifier_error = modifier.initialize(this->_modifier4);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (modifier_error);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_item::set_modifier4(const ft_item_modifier &mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    modifier_error = this->_modifier4.initialize(mod);
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier4_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_identifier;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_identifier = this->_modifier4.get_id();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_identifier);
}

void ft_item::set_modifier4_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier4.set_id(id);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_item::get_modifier4_value() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_value;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(lock_error);
        return (0);
    }
    modifier_value = this->_modifier4.get_value();
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_item *>(this)->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_item *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (modifier_value);
}

void ft_item::set_modifier4_value(int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    int modifier_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier4.set_value(value);
    modifier_error = FT_ERR_SUCCESS;
    if (modifier_error != FT_ERR_SUCCESS)
    {
        this->set_error(modifier_error);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_item::set_error(int err) const noexcept
{
    ft_item::_last_error = err;
    return ;
}

int ft_item::get_error() const noexcept
{
    return (ft_item::_last_error);
}

const char *ft_item::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
