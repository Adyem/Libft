#include "logger.hpp"
#include "logger_internal.hpp"
#include "../Libft/libft.hpp"

ft_log_context_guard::ft_log_context_guard() noexcept
    : _pushed_count(0), _active(false), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_log_context_guard::ft_log_context_guard(const s_log_field *fields,
        size_t field_count) noexcept
    : _pushed_count(0), _active(false), _error_code(FT_ERR_SUCCESSS)
{
    size_t pushed_count;

    this->set_error(FT_ERR_SUCCESSS);
    if (logger_context_push(fields, field_count, &pushed_count) != 0)
    {
        this->_active = false;
        this->_pushed_count = 0;
        this->set_error(ft_errno);
        return ;
    }
    this->_pushed_count = pushed_count;
    if (pushed_count != 0)
        this->_active = true;
    else
        this->_active = false;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_log_context_guard::~ft_log_context_guard() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    if (this->_active)
    {
        logger_context_pop(this->_pushed_count);
        if (ft_errno != FT_ERR_SUCCESSS)
        {
            this->_active = false;
            this->set_error(ft_errno);
            ft_errno = entry_errno;
            return ;
        }
    }
    this->_active = false;
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

ft_log_context_guard::ft_log_context_guard(ft_log_context_guard &&other) noexcept
    : _pushed_count(other._pushed_count), _active(other._active), _error_code(other._error_code)
{
    int entry_errno;

    entry_errno = ft_errno;
    other._pushed_count = 0;
    other._active = false;
    other._error_code = FT_ERR_SUCCESSS;
    other.set_error(FT_ERR_SUCCESSS);
    this->set_error(this->_error_code);
    ft_errno = entry_errno;
    return ;
}

ft_log_context_guard &ft_log_context_guard::operator=(ft_log_context_guard &&other) noexcept
{
    if (this != &other)
    {
        int entry_errno;

        entry_errno = ft_errno;
        if (this->_active)
        {
            logger_context_pop(this->_pushed_count);
            if (ft_errno != FT_ERR_SUCCESSS)
            {
                this->_active = false;
                this->set_error(ft_errno);
                ft_errno = entry_errno;
                return (*this);
            }
        }
        this->_pushed_count = other._pushed_count;
        this->_active = other._active;
        this->_error_code = other._error_code;
        other._pushed_count = 0;
        other._active = false;
        other._error_code = FT_ERR_SUCCESSS;
        other.set_error(FT_ERR_SUCCESSS);
        this->set_error(this->_error_code);
        ft_errno = entry_errno;
    }
    return (*this);
}

void ft_log_context_guard::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_log_context_guard::release() noexcept
{
    if (!this->_active)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    logger_context_pop(this->_pushed_count);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->_active = false;
        this->set_error(ft_errno);
        return ;
    }
    this->_active = false;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

bool ft_log_context_guard::is_active() const noexcept
{
    bool guard_active;

    guard_active = this->_active;
    const_cast<ft_log_context_guard *>(this)->set_error(FT_ERR_SUCCESSS);
    return (guard_active);
}

int ft_log_context_guard::get_error() const
{
    return (this->_error_code);
}

const char *ft_log_context_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

