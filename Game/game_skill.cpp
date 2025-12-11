#include "game_skill.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_skill_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_skill_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    (void)entry_errno;
    return ;
}

int ft_skill::lock_pair(const ft_skill &first, const ft_skill &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_skill *ordered_first;
    const ft_skill *ordered_second;
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
        const ft_skill *temporary;

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
        game_skill_sleep_backoff();
    }
}

ft_skill::ft_skill() noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0),
      _modifier3(0), _modifier4(0), _error(FT_ERR_SUCCESSS), _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_skill::~ft_skill() noexcept
{
    return ;
}

ft_skill::ft_skill(const ft_skill &other) noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0),
      _modifier3(0), _modifier4(0), _error(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_skill_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_level = other._level;
    this->_cooldown = other._cooldown;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_skill_restore_errno(other_guard, entry_errno);
    return ;
}

ft_skill &ft_skill::operator=(const ft_skill &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_skill::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_level = other._level;
    this->_cooldown = other._cooldown;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_skill_restore_errno(this_guard, entry_errno);
    game_skill_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_skill::ft_skill(ft_skill &&other) noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0),
      _modifier3(0), _modifier4(0), _error(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_skill_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_id = other._id;
    this->_level = other._level;
    this->_cooldown = other._cooldown;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._level = 0;
    other._cooldown = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other._error = FT_ERR_SUCCESSS;
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(other_guard, entry_errno);
    return ;
}

ft_skill &ft_skill::operator=(ft_skill &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_skill::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_level = other._level;
    this->_cooldown = other._cooldown;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._level = 0;
    other._cooldown = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other._error = FT_ERR_SUCCESSS;
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(this_guard, entry_errno);
    game_skill_restore_errno(other_guard, entry_errno);
    return (*this);
}

void ft_skill::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

int ft_skill::get_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_skill *>(this)->set_error(this->_error);
    game_skill_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_skill::set_id(int id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->_id = 0;
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_id = id;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_level() const noexcept
{
    int entry_errno;
    int level_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    level_value = this->_level;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (level_value);
}

void ft_skill::set_level(int level) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (level < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_level = level;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_cooldown() const noexcept
{
    int entry_errno;
    int cooldown_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    cooldown_value = this->_cooldown;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (cooldown_value);
}

void ft_skill::set_cooldown(int cooldown) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (cooldown < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_cooldown = cooldown;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::add_cooldown(int cooldown) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (cooldown < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_cooldown += cooldown;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::sub_cooldown(int cooldown) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (cooldown < 0 || this->_cooldown < cooldown)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_cooldown -= cooldown;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_modifier1() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier1;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_skill::set_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 = mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::add_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 += mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::sub_modifier1(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier1 -= mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_modifier2() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier2;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_skill::set_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 = mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::add_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 += mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::sub_modifier2(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier2 -= mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_modifier3() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier3;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_skill::set_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 = mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::add_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 += mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::sub_modifier3(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier3 -= mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_modifier4() const noexcept
{
    int entry_errno;
    int modifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (0);
    }
    modifier = this->_modifier4;
    const_cast<ft_skill *>(this)->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return (modifier);
}

void ft_skill::set_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 = mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::add_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 += mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

void ft_skill::sub_modifier4(int mod) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_skill_restore_errno(guard, entry_errno);
        return ;
    }
    this->_modifier4 -= mod;
    this->set_error(FT_ERR_SUCCESSS);
    game_skill_restore_errno(guard, entry_errno);
    return ;
}

int ft_skill::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_skill *>(this)->set_error(error_code);
    game_skill_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_skill::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_skill *>(this)->set_error(guard.get_error());
        game_skill_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_skill *>(this)->set_error(error_code);
    game_skill_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}
