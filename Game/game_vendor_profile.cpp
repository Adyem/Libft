#include "ft_vendor_profile.hpp"
#include "game_economy_helpers.hpp"
#include "../Template/move.hpp"
#include <new>

int ft_vendor_profile::lock_pair(const ft_vendor_profile &first, const ft_vendor_profile &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_vendor_profile *ordered_first;
    const ft_vendor_profile *ordered_second;
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
        const ft_vendor_profile *temporary;

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
        game_economy_sleep_backoff();
    }
}

ft_vendor_profile::ft_vendor_profile() noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(ER_SUCCESS)
{
    return ;
}

ft_vendor_profile::ft_vendor_profile(int vendor_id, double buy_markup, double sell_multiplier, double tax_rate) noexcept
    : _vendor_id(vendor_id), _buy_markup(buy_markup), _sell_multiplier(sell_multiplier), _tax_rate(tax_rate), _error_code(ER_SUCCESS)
{
    return ;
}

ft_vendor_profile::ft_vendor_profile(const ft_vendor_profile &other) noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    entry_errno = ft_errno;
    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_vendor_profile &ft_vendor_profile::operator=(const ft_vendor_profile &other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_vendor_profile::ft_vendor_profile(ft_vendor_profile &&other) noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_economy_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    other._vendor_id = 0;
    other._buy_markup = 1.0;
    other._sell_multiplier = 1.0;
    other._tax_rate = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(other_guard, entry_errno);
    return ;
}

ft_vendor_profile &ft_vendor_profile::operator=(ft_vendor_profile &&other) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != ER_SUCCESS)
    {
        this->set_error(self_guard.get_error());
        game_economy_restore_errno(self_guard, entry_errno);
        game_economy_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    other._vendor_id = 0;
    other._buy_markup = 1.0;
    other._sell_multiplier = 1.0;
    other._tax_rate = 0.0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_economy_restore_errno(self_guard, entry_errno);
    game_economy_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_vendor_profile::get_vendor_id() const noexcept
{
    int entry_errno;
    int identifier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    identifier = this->_vendor_id;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (identifier);
}

void ft_vendor_profile::set_vendor_id(int vendor_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_vendor_id = vendor_id;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

double ft_vendor_profile::get_buy_markup() const noexcept
{
    int entry_errno;
    double markup;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (static_cast<double>(guard.get_error()));
    }
    markup = this->_buy_markup;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (markup);
}

void ft_vendor_profile::set_buy_markup(double buy_markup) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_buy_markup = buy_markup;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

double ft_vendor_profile::get_sell_multiplier() const noexcept
{
    int entry_errno;
    double multiplier;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (static_cast<double>(guard.get_error()));
    }
    multiplier = this->_sell_multiplier;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (multiplier);
}

void ft_vendor_profile::set_sell_multiplier(double sell_multiplier) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_sell_multiplier = sell_multiplier;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

double ft_vendor_profile::get_tax_rate() const noexcept
{
    int entry_errno;
    double tax_rate;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (static_cast<double>(guard.get_error()));
    }
    tax_rate = this->_tax_rate;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    game_economy_restore_errno(guard, entry_errno);
    return (tax_rate);
}

void ft_vendor_profile::set_tax_rate(double tax_rate) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return ;
    }
    this->_tax_rate = tax_rate;
    this->set_error(ER_SUCCESS);
    game_economy_restore_errno(guard, entry_errno);
    return ;
}

int ft_vendor_profile::get_error() const noexcept
{
    int entry_errno;
    int error_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        game_economy_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_vendor_profile *>(this)->set_error(error_value);
    game_economy_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_vendor_profile::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_vendor_profile::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}
