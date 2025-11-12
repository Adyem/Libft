#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include <utility>

static void game_hooks_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_hooks_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

template <typename Callback, typename... Args>
static void game_hooks_invoke_safe(const ft_game_hooks *hooks,
    Callback &callback,
    ft_unique_lock<pt_mutex> &guard,
    int entry_errno,
    Args &...args) noexcept
{
    if (callback.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(hooks)->set_error(callback.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    if (callback)
        callback(args...);
    return ;
}

int ft_game_hooks::lock_pair(const ft_game_hooks &first, const ft_game_hooks &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_game_hooks *ordered_first;
    const ft_game_hooks *ordered_second;
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
        const ft_game_hooks *temporary;

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
        game_hooks_sleep_backoff();
    }
}

ft_game_hooks::ft_game_hooks() noexcept
    : _on_item_crafted(), _on_character_damaged(), _on_event_triggered(),
      _error_code(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_game_hooks::~ft_game_hooks() noexcept
{
    return ;
}

ft_game_hooks::ft_game_hooks(const ft_game_hooks &other) noexcept
    : _on_item_crafted(), _on_character_damaged(), _on_event_triggered(),
      _error_code(other._error_code), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_item_crafted = other._on_item_crafted;
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_character_damaged = other._on_character_damaged;
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_event_triggered = other._on_event_triggered;
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->set_error(this->_error_code);
    game_hooks_restore_errno(other_guard, entry_errno);
    return ;
}

ft_game_hooks &ft_game_hooks::operator=(const ft_game_hooks &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_game_hooks::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_item_crafted = other._on_item_crafted;
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_character_damaged = other._on_character_damaged;
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_event_triggered = other._on_event_triggered;
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    ft_errno = entry_errno;
    return (*this);
}

ft_game_hooks::ft_game_hooks(ft_game_hooks &&other) noexcept
    : _on_item_crafted(), _on_character_damaged(), _on_event_triggered(),
      _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_item_crafted = ft_move(other._on_item_crafted);
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_character_damaged = ft_move(other._on_character_damaged);
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_on_event_triggered = ft_move(other._on_event_triggered);
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        game_hooks_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other._error_code;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    game_hooks_restore_errno(other_guard, entry_errno);
    return ;
}

ft_game_hooks &ft_game_hooks::operator=(ft_game_hooks &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_game_hooks::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_item_crafted = ft_move(other._on_item_crafted);
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_character_damaged = ft_move(other._on_character_damaged);
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_on_event_triggered = ft_move(other._on_event_triggered);
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        ft_errno = entry_errno;
        return (*this);
    }
    this->_error_code = other._error_code;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    ft_errno = entry_errno;
    return (*this);
}

void ft_game_hooks::set_on_item_crafted(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_item_crafted = ft_move(callback);
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return ;
}

void ft_game_hooks::set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_character_damaged = ft_move(callback);
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return ;
}

void ft_game_hooks::set_on_event_triggered(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_event_triggered = ft_move(callback);
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return ;
}

ft_function<void(ft_character&, ft_item&)> ft_game_hooks::get_on_item_crafted() const noexcept
{
    int entry_errno;
    ft_function<void(ft_character&, ft_item&)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_character&, ft_item&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    callback = this->_on_item_crafted;
    if (callback.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return (callback);
}

ft_function<void(ft_character&, int, uint8_t)> ft_game_hooks::get_on_character_damaged() const noexcept
{
    int entry_errno;
    ft_function<void(ft_character&, int, uint8_t)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_character&, int, uint8_t)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    callback = this->_on_character_damaged;
    if (callback.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return (callback);
}

ft_function<void(ft_world&, ft_event&)> ft_game_hooks::get_on_event_triggered() const noexcept
{
    int entry_errno;
    ft_function<void(ft_world&, ft_event&)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_world&, ft_event&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    callback = this->_on_event_triggered;
    if (callback.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return (callback);
}

void ft_game_hooks::invoke_on_item_crafted(ft_character &character, ft_item &item) const noexcept
{
    int entry_errno;
    ft_function<void(ft_character&, ft_item&)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_character&, ft_item&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    callback = this->_on_item_crafted;
    game_hooks_invoke_safe(this, callback, guard, entry_errno, character, item);
    return ;
}

void ft_game_hooks::invoke_on_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept
{
    int entry_errno;
    ft_function<void(ft_character&, int, uint8_t)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_character&, int, uint8_t)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    callback = this->_on_character_damaged;
    game_hooks_invoke_safe(this, callback, guard, entry_errno, character, damage, type);
    return ;
}

void ft_game_hooks::invoke_on_event_triggered(ft_world &world, ft_event &event) const noexcept
{
    int entry_errno;
    ft_function<void(ft_world&, ft_event&)> callback;

    entry_errno = ft_errno;
    callback = ft_function<void(ft_world&, ft_event&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    callback = this->_on_event_triggered;
    game_hooks_invoke_safe(this, callback, guard, entry_errno, world, event);
    return ;
}

void ft_game_hooks::reset() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_item_crafted = ft_function<void(ft_character&, ft_item&)>();
    if (this->_on_item_crafted.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_item_crafted.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_character_damaged = ft_function<void(ft_character&, int, uint8_t)>();
    if (this->_on_character_damaged.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_character_damaged.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->_on_event_triggered = ft_function<void(ft_world&, ft_event&)>();
    if (this->_on_event_triggered.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_on_event_triggered.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_hooks_restore_errno(guard, entry_errno);
    return ;
}

int ft_game_hooks::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_game_hooks *>(this)->set_error(error_code);
    game_hooks_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_game_hooks::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        game_hooks_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_game_hooks *>(this)->set_error(error_code);
    game_hooks_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_game_hooks::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}
