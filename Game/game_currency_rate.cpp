#include "ft_currency_rate.hpp"
#include "../Template/move.hpp"

thread_local ft_operation_error_stack ft_currency_rate::_operation_errors = {{}, {}, 0};

void ft_currency_rate::record_operation_error_unlocked(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(ft_currency_rate::_operation_errors,
            error_code, operation_id);
    return ;
}

int ft_currency_rate::lock_pair(const ft_currency_rate &first, const ft_currency_rate &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_currency_rate *ordered_first;
    const ft_currency_rate *ordered_second;
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
        const ft_currency_rate *temporary;

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

ft_currency_rate::ft_currency_rate() noexcept
    : _currency_id(0), _rate_to_base(1.0), _display_precision(2),
    _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_currency_rate::ft_currency_rate(int currency_id, double rate_to_base, int display_precision) noexcept
    : _currency_id(currency_id), _rate_to_base(rate_to_base), _display_precision(display_precision), _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_currency_rate::ft_currency_rate(const ft_currency_rate &other) noexcept
    : _currency_id(0), _rate_to_base(1.0), _display_precision(2), _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_currency_rate::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_currency_id = other._currency_id;
    this->_rate_to_base = other._rate_to_base;
    this->_display_precision = other._display_precision;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return ;
}

ft_currency_rate &ft_currency_rate::operator=(const ft_currency_rate &other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_currency_rate::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_currency_id = other._currency_id;
    this->_rate_to_base = other._rate_to_base;
    this->_display_precision = other._display_precision;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    return (*this);
}

ft_currency_rate::ft_currency_rate(ft_currency_rate &&other) noexcept
    : _currency_id(0), _rate_to_base(1.0), _display_precision(2), _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (ft_currency_rate::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return ;
    }
    this->_currency_id = other._currency_id;
    this->_rate_to_base = other._rate_to_base;
    this->_display_precision = other._display_precision;
    this->_error_code = other._error_code;
    other._currency_id = 0;
    other._rate_to_base = 0.0;
    other._display_precision = 0;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_currency_rate &ft_currency_rate::operator=(ft_currency_rate &&other) noexcept
{
    ft_unique_lock<pt_mutex> self_guard;
    ft_unique_lock<pt_mutex> other_guard;

    if (this == &other)
        return (*this);
    if (ft_currency_rate::lock_pair(*this, other, self_guard, other_guard) != FT_ERR_SUCCESSS)
    {
        this->set_error(self_guard.get_error());
        return (*this);
    }
    this->_currency_id = other._currency_id;
    this->_rate_to_base = other._rate_to_base;
    this->_display_precision = other._display_precision;
    this->_error_code = other._error_code;
    other._currency_id = 0;
    other._rate_to_base = 0.0;
    other._display_precision = 0;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESSS);
    return (*this);
}

int ft_currency_rate::get_currency_id() const noexcept
{
    int identifier;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_currency_rate *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    identifier = this->_currency_id;
    const_cast<ft_currency_rate *>(this)->set_error(this->_error_code);
    return (identifier);
}

void ft_currency_rate::set_currency_id(int currency_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_currency_id = currency_id;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

double ft_currency_rate::get_rate_to_base() const noexcept
{
    double rate_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_currency_rate *>(this)->set_error(guard.get_error());
        return (static_cast<double>(guard.get_error()));
    }
    rate_value = this->_rate_to_base;
    const_cast<ft_currency_rate *>(this)->set_error(this->_error_code);
    return (rate_value);
}

void ft_currency_rate::set_rate_to_base(double rate_to_base) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_rate_to_base = rate_to_base;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_currency_rate::get_display_precision() const noexcept
{
    int precision;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_currency_rate *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    precision = this->_display_precision;
    const_cast<ft_currency_rate *>(this)->set_error(this->_error_code);
    return (precision);
}

void ft_currency_rate::set_display_precision(int display_precision) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_display_precision = display_precision;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_currency_rate::get_error() const noexcept
{
    int error_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_currency_rate *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_value = this->_error_code;
    const_cast<ft_currency_rate *>(this)->set_error(error_value);
    return (error_value);
}

const char *ft_currency_rate::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

void ft_currency_rate::set_error(int error_code) const noexcept
{

    this->_error_code = error_code;
    ft_currency_rate::record_operation_error_unlocked(error_code);
    return ;
}
