#include "ft_rarity_band.hpp"
#include "game_economy_helpers.hpp"
#include "../Template/move.hpp"

int ft_rarity_band::lock_pair(const ft_rarity_band &first, const ft_rarity_band &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_rarity_band *ordered_first;
    const ft_rarity_band *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_rarity_band *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_economy_sleep_backoff();
    }
}

ft_rarity_band::ft_rarity_band() noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_rarity_band::ft_rarity_band(int rarity, double value_multiplier) noexcept
    : _rarity(rarity), _value_multiplier(value_multiplier), _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_rarity_band::ft_rarity_band(const ft_rarity_band &other) noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ER_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_rarity_band &ft_rarity_band::operator=(const ft_rarity_band &other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ER_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_rarity_band::ft_rarity_band(ft_rarity_band &&other) noexcept
    : _rarity(0), _value_multiplier(1.0), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ER_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    other._rarity = 0;
    other._value_multiplier = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_rarity_band &ft_rarity_band::operator=(ft_rarity_band &&other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    if (ft_rarity_band::lock_pair(*this, other, self_guard, other_guard) != FT_ER_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->_error_code = other._error_code;
    other._rarity = 0;
    other._value_multiplier = 0.0;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_rarity_band::get_rarity() const noexcept
{
    int entry_errno;
    int rarity_value;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    rarity_value = this->_rarity;
    const_cast<ft_rarity_band *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (rarity_value);
}

void ft_rarity_band::set_rarity(int rarity) noexcept
{
    int entry_errno;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_rarity = rarity;
    this->set_error(FT_ER_SUCCESSS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

double ft_rarity_band::get_value_multiplier() const noexcept
{
    int entry_errno;
    double multiplier;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (static_cast<double>(guard.get_error()));
    }
    multiplier = this->_value_multiplier;
    const_cast<ft_rarity_band *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (multiplier);
}

void ft_rarity_band::set_value_multiplier(double value_multiplier) noexcept
{
    int entry_errno;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_value_multiplier = value_multiplier;
    this->set_error(FT_ER_SUCCESSS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_rarity_band::get_error() const noexcept
{
    int entry_errno;
    int error_value;

    ft_errno = FT_ER_SUCCESSS;
    entry_errno = FT_ER_SUCCESSS;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_rarity_band *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_rarity_band *>(this)->set_error(error_value);
    game_economy_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_rarity_band::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_rarity_band::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}
