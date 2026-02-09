#include "ft_vendor_profile.hpp"
#include "../Template/move.hpp"

thread_local ft_operation_error_stack ft_vendor_profile::_operation_errors = {{}, {}, 0};

void ft_vendor_profile::record_operation_error_unlocked(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(ft_vendor_profile::_operation_errors,
            error_code, operation_id);
    return ;
}

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

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESSS);
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

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
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
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_vendor_profile::ft_vendor_profile() noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0),
    _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_vendor_profile::ft_vendor_profile(int vendor_id, double buy_markup, double sell_multiplier, double tax_rate) noexcept
    : _vendor_id(vendor_id), _buy_markup(buy_markup), _sell_multiplier(sell_multiplier), _tax_rate(tax_rate), _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_vendor_profile::ft_vendor_profile(const ft_vendor_profile &other) noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return ;
}

ft_vendor_profile &ft_vendor_profile::operator=(const ft_vendor_profile &other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return (*this);
}

ft_vendor_profile::ft_vendor_profile(ft_vendor_profile &&other) noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0), _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
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
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_vendor_profile &ft_vendor_profile::operator=(ft_vendor_profile &&other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_vendor_profile::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
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
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return (*this);
}

int ft_vendor_profile::get_vendor_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    identifier = this->_vendor_id;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    return (identifier);
}

void ft_vendor_profile::set_vendor_id(int vendor_id) noexcept
{
    if (vendor_id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_vendor_id = vendor_id;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

double ft_vendor_profile::get_buy_markup() const noexcept
{
    double markup;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        return (static_cast<double>(guard.get_error()));
    }
    markup = this->_buy_markup;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    return (markup);
}

void ft_vendor_profile::set_buy_markup(double buy_markup) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_buy_markup = buy_markup;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

double ft_vendor_profile::get_sell_multiplier() const noexcept
{
    double multiplier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        return (static_cast<double>(guard.get_error()));
    }
    multiplier = this->_sell_multiplier;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    return (multiplier);
}

void ft_vendor_profile::set_sell_multiplier(double sell_multiplier) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_sell_multiplier = sell_multiplier;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

double ft_vendor_profile::get_tax_rate() const noexcept
{
    double tax_rate;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        return (static_cast<double>(guard.get_error()));
    }
    tax_rate = this->_tax_rate;
    const_cast<ft_vendor_profile *>(this)->set_error(this->_error_code);
    return (tax_rate);
}

void ft_vendor_profile::set_tax_rate(double tax_rate) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_tax_rate = tax_rate;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_vendor_profile::get_error() const noexcept
{
    int error_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_vendor_profile *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_vendor_profile *>(this)->set_error(error_value);
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
    this->_error_code = error_code;
    ft_vendor_profile::record_operation_error_unlocked(error_code);
    return ;
}
