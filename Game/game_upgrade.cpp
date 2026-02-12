#include "game_upgrade.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_upgrade_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_upgrade_finalize_lock(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESS)
        ft_errno = guard.get_error();
    return ;
}

int ft_upgrade::lock_pair(const ft_upgrade &first, const ft_upgrade &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_upgrade *ordered_first;
    const ft_upgrade *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESS;
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_upgrade *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
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
            ft_errno = FT_ERR_SUCCESS;
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_upgrade_sleep_backoff();
    }
}

ft_upgrade::ft_upgrade() noexcept
    : _id(0), _current_level(0), _max_level(0),
      _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0),
      _error(FT_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_upgrade::~ft_upgrade() noexcept
{
    return ;
}

ft_upgrade::ft_upgrade(const ft_upgrade &other) noexcept
    : _id(0), _current_level(0), _max_level(0),
      _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0),
      _error(FT_ERR_SUCCESS), _mutex()
{

    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_upgrade_finalize_lock(other_guard);
        return ;
    }
    this->_id = other._id;
    this->_current_level = other._current_level;
    this->_max_level = other._max_level;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_upgrade_finalize_lock(other_guard);
    return ;
}

ft_upgrade &ft_upgrade::operator=(const ft_upgrade &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_upgrade::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_current_level = other._current_level;
    this->_max_level = other._max_level;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    this->set_error(other._error);
    game_upgrade_finalize_lock(this_guard);
    game_upgrade_finalize_lock(other_guard);
    return (*this);
}

ft_upgrade::ft_upgrade(ft_upgrade &&other) noexcept
    : _id(0), _current_level(0), _max_level(0),
      _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0),
      _error(FT_ERR_SUCCESS), _mutex()
{

    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_upgrade_finalize_lock(other_guard);
        return ;
    }
    this->_id = other._id;
    this->_current_level = other._current_level;
    this->_max_level = other._max_level;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._current_level = 0;
    other._max_level = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other.set_error(FT_ERR_SUCCESS);
    this->set_error(this->_error);
    game_upgrade_finalize_lock(other_guard);
    return ;
}

ft_upgrade &ft_upgrade::operator=(ft_upgrade &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_upgrade::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_current_level = other._current_level;
    this->_max_level = other._max_level;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_error = other._error;
    other._id = 0;
    other._current_level = 0;
    other._max_level = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other.set_error(FT_ERR_SUCCESS);
    this->set_error(this->_error);
    game_upgrade_finalize_lock(this_guard);
    game_upgrade_finalize_lock(other_guard);
    return (*this);
}

int ft_upgrade::get_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    identifier = this->_id;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (identifier);
}

void ft_upgrade::set_id(int id) noexcept
{

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_id = id;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

uint16_t ft_upgrade::get_current_level() const noexcept
{
    uint16_t level;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    level = this->_current_level;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (level);
}

void ft_upgrade::set_current_level(uint16_t level) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    if (this->_max_level != 0 && level > this->_max_level)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_current_level = level;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::add_level(uint16_t level) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_current_level += level;
    if (this->_max_level != 0 && this->_current_level > this->_max_level)
        this->_current_level = this->_max_level;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::sub_level(uint16_t level) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    if (level > this->_current_level)
        this->_current_level = 0;
    else
        this->_current_level -= level;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

uint16_t ft_upgrade::get_max_level() const noexcept
{
    uint16_t level;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    level = this->_max_level;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (level);
}

void ft_upgrade::set_max_level(uint16_t level) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_max_level = level;
    if (this->_max_level != 0 && this->_current_level > this->_max_level)
        this->_current_level = this->_max_level;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

int ft_upgrade::get_modifier1() const noexcept
{
    int modifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    modifier = this->_modifier1;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (modifier);
}

void ft_upgrade::set_modifier1(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier1 = mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::add_modifier1(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier1 += mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::sub_modifier1(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier1 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

int ft_upgrade::get_modifier2() const noexcept
{
    int modifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    modifier = this->_modifier2;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (modifier);
}

void ft_upgrade::set_modifier2(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier2 = mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::add_modifier2(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier2 += mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::sub_modifier2(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier2 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

int ft_upgrade::get_modifier3() const noexcept
{
    int modifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    modifier = this->_modifier3;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (modifier);
}

void ft_upgrade::set_modifier3(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier3 = mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::add_modifier3(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier3 += mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::sub_modifier3(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier3 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

int ft_upgrade::get_modifier4() const noexcept
{
    int modifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (0);
    }
    modifier = this->_modifier4;
    const_cast<ft_upgrade *>(this)->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return (modifier);
}

void ft_upgrade::set_modifier4(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier4 = mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::add_modifier4(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier4 += mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

void ft_upgrade::sub_modifier4(int mod) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return ;
    }
    this->_modifier4 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    game_upgrade_finalize_lock(guard);
    return ;
}

int ft_upgrade::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_upgrade *>(this)->set_error(error_code);
    game_upgrade_finalize_lock(guard);
    return (error_code);
}

const char *ft_upgrade::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_upgrade *>(this)->set_error(guard.get_error());
        game_upgrade_finalize_lock(guard);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_upgrade *>(this)->set_error(error_code);
    game_upgrade_finalize_lock(guard);
    return (ft_strerror(error_code));
}

void ft_upgrade::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
