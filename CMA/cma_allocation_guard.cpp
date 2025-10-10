#include "cma_allocation_guard.hpp"

cma_allocation_guard::cma_allocation_guard()
    : _pointer(ft_nullptr), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

cma_allocation_guard::cma_allocation_guard(void *memory_pointer)
    : _pointer(memory_pointer), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

cma_allocation_guard::~cma_allocation_guard()
{
    if (this->_pointer != ft_nullptr)
    {
        cma_free(this->_pointer);
        this->_pointer = ft_nullptr;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

cma_allocation_guard::cma_allocation_guard(cma_allocation_guard &&other) noexcept
    : _pointer(other._pointer), _error_code(other._error_code)
{
    other._pointer = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    return ;
}

cma_allocation_guard &cma_allocation_guard::operator=(cma_allocation_guard &&other) noexcept
{
    if (this != &other)
    {
        if (this->_pointer != ft_nullptr)
        {
            cma_free(this->_pointer);
        }
        this->_pointer = other._pointer;
        this->_error_code = other._error_code;
        other._pointer = ft_nullptr;
        other._error_code = ER_SUCCESS;
        other.set_error(ER_SUCCESS);
        this->set_error(this->_error_code);
    }
    return (*this);
}

void cma_allocation_guard::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void cma_allocation_guard::reset(void *memory_pointer)
{
    if (this->_pointer == memory_pointer)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (this->_pointer != ft_nullptr)
    {
        cma_free(this->_pointer);
    }
    this->_pointer = memory_pointer;
    this->set_error(ER_SUCCESS);
    return ;
}

void *cma_allocation_guard::release()
{
    void *released_pointer;

    released_pointer = this->_pointer;
    this->_pointer = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return (released_pointer);
}

void *cma_allocation_guard::get() const
{
    this->set_error(ER_SUCCESS);
    return (this->_pointer);
}

bool cma_allocation_guard::owns_allocation() const
{
    bool owns_memory;

    owns_memory = (this->_pointer != ft_nullptr);
    const_cast<cma_allocation_guard *>(this)->set_error(ER_SUCCESS);
    return (owns_memory);
}

int cma_allocation_guard::get_error() const
{
    return (this->_error_code);
}

const char *cma_allocation_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

cma_allocation_guard cma_make_allocation_guard(void *memory_pointer)
{
    cma_allocation_guard allocation_guard(memory_pointer);

    return (allocation_guard);
}
