#include "ft_vendor_profile.hpp"
#include "game_economy_helpers.hpp"
#include <new>

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

ft_vendor_profile::ft_vendor_profile(ft_vendor_profile &&other) noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(ER_SUCCESS)
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    other._mutex.lock(THREAD_ID);
    lock_error = other._mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    lock_acquired = true;
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
    restore_error = game_economy_restore_errno(other._mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return ;
}

ft_vendor_profile &ft_vendor_profile::operator=(ft_vendor_profile &&other) noexcept
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    other._mutex.lock(THREAD_ID);
    lock_error = other._mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    lock_acquired = true;
    this->_mutex.~pt_mutex();
    new (&this->_mutex) pt_mutex();
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
    restore_error = game_economy_restore_errno(other._mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return (*this);
}

int ft_vendor_profile::get_vendor_id() const noexcept
{
    int entry_errno;
    int identifier;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    identifier = 0;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(lock_error);
        return (lock_error);
    }
    lock_acquired = true;
    identifier = this->_vendor_id;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(restore_error);
        return (restore_error);
    }
    return (identifier);
}

void ft_vendor_profile::set_vendor_id(int vendor_id) noexcept
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    lock_acquired = true;
    this->_vendor_id = vendor_id;
    this->set_error(ER_SUCCESS);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return ;
}

double ft_vendor_profile::get_buy_markup() const noexcept
{
    int entry_errno;
    double markup;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    markup = 0.0;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(lock_error);
        return (static_cast<double>(lock_error));
    }
    lock_acquired = true;
    markup = this->_buy_markup;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(restore_error);
        return (static_cast<double>(restore_error));
    }
    return (markup);
}

void ft_vendor_profile::set_buy_markup(double buy_markup) noexcept
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    lock_acquired = true;
    this->_buy_markup = buy_markup;
    this->set_error(ER_SUCCESS);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return ;
}

double ft_vendor_profile::get_sell_multiplier() const noexcept
{
    int entry_errno;
    double multiplier;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    multiplier = 0.0;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(lock_error);
        return (static_cast<double>(lock_error));
    }
    lock_acquired = true;
    multiplier = this->_sell_multiplier;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(restore_error);
        return (static_cast<double>(restore_error));
    }
    return (multiplier);
}

void ft_vendor_profile::set_sell_multiplier(double sell_multiplier) noexcept
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    lock_acquired = true;
    this->_sell_multiplier = sell_multiplier;
    this->set_error(ER_SUCCESS);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return ;
}

double ft_vendor_profile::get_tax_rate() const noexcept
{
    int entry_errno;
    double tax_rate;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    tax_rate = 0.0;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(lock_error);
        return (static_cast<double>(lock_error));
    }
    lock_acquired = true;
    tax_rate = this->_tax_rate;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(restore_error);
        return (static_cast<double>(restore_error));
    }
    return (tax_rate);
}

void ft_vendor_profile::set_tax_rate(double tax_rate) noexcept
{
    int entry_errno;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    lock_acquired = true;
    this->_tax_rate = tax_rate;
    this->set_error(ER_SUCCESS);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
        this->set_error(restore_error);
    return ;
}

int ft_vendor_profile::get_error() const noexcept
{
    int entry_errno;
    int error_value;
    int lock_error;
    int restore_error;
    bool lock_acquired;

    entry_errno = ft_errno;
    error_value = ER_SUCCESS;
    lock_error = ER_SUCCESS;
    restore_error = ER_SUCCESS;
    lock_acquired = false;
    this->_mutex.lock(THREAD_ID);
    lock_error = this->_mutex.get_error();
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(lock_error);
        return (lock_error);
    }
    lock_acquired = true;
    error_value = this->_error_code;
    const_cast<ft_vendor_profile *>(this)->set_error(error_value);
    restore_error = game_economy_restore_errno(this->_mutex, entry_errno, lock_acquired);
    if (restore_error != ER_SUCCESS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(restore_error);
        return (restore_error);
    }
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
