#include "cma_alloc_limit_guard.hpp"
#include "cma_internal.hpp"

cma_alloc_limit_guard::cma_alloc_limit_guard(ft_size_t new_limit)
    : _previous_limit(0), _active(false), _error_code(FT_ERR_SUCCESSS)
{
    int entry_errno;
    int set_limit_error;
    cma_allocator_guard allocator_guard;

    entry_errno = ft_errno;
    if (!allocator_guard.is_active())
    {
        this->_previous_limit = g_cma_alloc_limit;
        cma_set_alloc_limit(new_limit);
        set_limit_error = ft_errno;
        if (set_limit_error != FT_ERR_SUCCESSS)
        {
            this->_active = false;
            this->set_error(set_limit_error);
            return ;
        }
        this->_active = true;
        this->set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
        return ;
    }
    this->_previous_limit = g_cma_alloc_limit;
    ft_errno = entry_errno;
    g_cma_alloc_limit = new_limit;
    set_limit_error = ft_errno;
    allocator_guard.unlock();
    ft_errno = set_limit_error;
    this->_active = true;
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

cma_alloc_limit_guard::~cma_alloc_limit_guard()
{
    int entry_errno;
    int restore_error;

    entry_errno = ft_errno;
    if (!this->_active)
    {
        this->set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
        return ;
    }
    {
        cma_allocator_guard allocator_guard;

        if (!allocator_guard.is_active())
        {
            cma_set_alloc_limit(this->_previous_limit);
            restore_error = ft_errno;
            this->_active = false;
            this->set_error(restore_error);
            if (restore_error == FT_ERR_SUCCESSS)
                ft_errno = entry_errno;
            return ;
        }
        ft_errno = entry_errno;
        g_cma_alloc_limit = this->_previous_limit;
        restore_error = ft_errno;
        allocator_guard.unlock();
        ft_errno = restore_error;
    }
    this->_active = false;
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

cma_alloc_limit_guard::cma_alloc_limit_guard(cma_alloc_limit_guard &&other) noexcept
    : _previous_limit(other._previous_limit), _active(other._active), _error_code(other._error_code)
{
    int entry_errno;

    entry_errno = ft_errno;
    other._previous_limit = 0;
    other._active = false;
    other._error_code = FT_ERR_SUCCESSS;
    other.set_error(FT_ERR_SUCCESSS);
    this->set_error(this->_error_code);
    ft_errno = entry_errno;
    return ;
}

cma_alloc_limit_guard &cma_alloc_limit_guard::operator=(cma_alloc_limit_guard &&other) noexcept
{
    if (this != &other)
    {
        int entry_errno;
        int restore_error;

        entry_errno = ft_errno;
        if (this->_active)
        {
            cma_set_alloc_limit(this->_previous_limit);
            restore_error = ft_errno;
            if (restore_error != FT_ERR_SUCCESSS)
            {
                this->_active = false;
                this->set_error(restore_error);
                return (*this);
            }
        }
        this->_previous_limit = other._previous_limit;
        this->_active = other._active;
        this->_error_code = other._error_code;
        other._previous_limit = 0;
        other._active = false;
        other._error_code = FT_ERR_SUCCESSS;
        other.set_error(FT_ERR_SUCCESSS);
        this->set_error(this->_error_code);
        ft_errno = entry_errno;
    }
    return (*this);
}

void cma_alloc_limit_guard::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void cma_alloc_limit_guard::reset(ft_size_t new_limit)
{
    int entry_errno;
    int operation_error;
    cma_allocator_guard allocator_guard;

    entry_errno = ft_errno;
    if (!allocator_guard.is_active())
    {
        if (this->_active)
        {
            cma_set_alloc_limit(this->_previous_limit);
            operation_error = ft_errno;
            if (operation_error != FT_ERR_SUCCESSS)
            {
                this->_active = false;
                this->set_error(operation_error);
                return ;
            }
        }
        this->_previous_limit = g_cma_alloc_limit;
        cma_set_alloc_limit(new_limit);
        operation_error = ft_errno;
        if (operation_error != FT_ERR_SUCCESSS)
        {
            this->_active = false;
            this->set_error(operation_error);
            return ;
        }
        this->_active = true;
        this->set_error(FT_ERR_SUCCESSS);
        ft_errno = entry_errno;
        return ;
    }
    if (this->_active)
    {
        ft_errno = entry_errno;
        g_cma_alloc_limit = this->_previous_limit;
    }
    this->_previous_limit = g_cma_alloc_limit;
    g_cma_alloc_limit = new_limit;
    operation_error = ft_errno;
    allocator_guard.unlock();
    ft_errno = operation_error;
    this->_active = true;
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

bool cma_alloc_limit_guard::is_active() const
{
    int entry_errno;
    bool guard_active;

    entry_errno = ft_errno;
    guard_active = this->_active;
    const_cast<cma_alloc_limit_guard *>(this)->set_error(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return (guard_active);
}

int cma_alloc_limit_guard::get_error() const
{
    return (this->_error_code);
}

const char *cma_alloc_limit_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

cma_alloc_limit_guard cma_make_alloc_limit_guard(ft_size_t new_limit)
{
    cma_alloc_limit_guard limit_guard(new_limit);

    return (limit_guard);
}
