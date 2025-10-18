#include "cma_allocation_guard.hpp"
#include "cma_internal.hpp"
#include "../Logger/logger.hpp"
#include "../System_utils/system_utils.hpp"

static int cma_allocation_guard_release_locked(void *memory_pointer,
        bool *invalid_state_detected)
{
    Block *block_pointer;
    Page *page_pointer;
    ft_size_t released_size;

    *invalid_state_detected = false;
    block_pointer = cma_find_block_for_pointer(memory_pointer);
    if (!block_pointer)
        return (FT_ERR_INVALID_POINTER);
    cma_validate_block(block_pointer, "cma_allocation_guard", memory_pointer);
    if (cma_block_is_free(block_pointer))
    {
        *invalid_state_detected = true;
        return (FT_ERR_INVALID_STATE);
    }
    released_size = block_pointer->size;
    cma_mark_block_free(block_pointer);
    block_pointer = merge_block(block_pointer);
    page_pointer = find_page_of_block(block_pointer);
    free_page_if_empty(page_pointer);
    if (g_cma_current_bytes >= released_size)
        g_cma_current_bytes -= released_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count += 1;
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

static int cma_allocation_guard_release_allocation(void *memory_pointer,
        bool *invalid_state_detected)
{
    cma_allocator_guard allocator_guard;
    int release_error;

    *invalid_state_detected = false;
    if (memory_pointer == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(memory_pointer))
    {
        cma_backend_deallocate(memory_pointer);
        release_error = ft_errno;
        return (release_error);
    }
    if (!allocator_guard.is_active())
    {
        cma_free(memory_pointer);
        release_error = ft_errno;
        return (release_error);
    }
    release_error = cma_allocation_guard_release_locked(memory_pointer,
            invalid_state_detected);
    ft_errno = release_error;
    allocator_guard.unlock();
    ft_errno = release_error;
    return (release_error);
}

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
    int entry_errno;
    int release_error;
    bool invalid_state_detected;
    void *managed_pointer;

    entry_errno = ft_errno;
    managed_pointer = this->_pointer;
    this->_pointer = ft_nullptr;
    release_error = cma_allocation_guard_release_allocation(managed_pointer,
            &invalid_state_detected);
    if (invalid_state_detected)
    {
        this->set_error(release_error);
        su_sigabrt();
    }
    if (managed_pointer != ft_nullptr && release_error == ER_SUCCESS
        && ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", managed_pointer);
    this->set_error(release_error);
    if (release_error == ER_SUCCESS)
        ft_errno = entry_errno;
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
        int entry_errno;
        void *previous_pointer;
        int release_error;
        bool invalid_state_detected;

        entry_errno = ft_errno;
        previous_pointer = this->_pointer;
        if (previous_pointer != ft_nullptr)
        {
            release_error = cma_allocation_guard_release_allocation(previous_pointer,
                    &invalid_state_detected);
            if (invalid_state_detected)
            {
                this->set_error(release_error);
                su_sigabrt();
            }
            if (release_error != ER_SUCCESS)
            {
                this->set_error(release_error);
                return (*this);
            }
            if (ft_log_get_alloc_logging())
                ft_log_debug("cma_free %p", previous_pointer);
        }
        this->_pointer = other._pointer;
        this->_error_code = other._error_code;
        other._pointer = ft_nullptr;
        other._error_code = ER_SUCCESS;
        other.set_error(ER_SUCCESS);
        this->set_error(this->_error_code);
        if (this->_error_code == ER_SUCCESS)
            ft_errno = entry_errno;
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
    int entry_errno;
    void *previous_pointer;
    int release_error;
    bool invalid_state_detected;

    entry_errno = ft_errno;
    if (this->_pointer == memory_pointer)
    {
        this->set_error(ER_SUCCESS);
        ft_errno = entry_errno;
        return ;
    }
    previous_pointer = this->_pointer;
    if (previous_pointer != ft_nullptr)
    {
        release_error = cma_allocation_guard_release_allocation(previous_pointer,
                &invalid_state_detected);
        if (invalid_state_detected)
        {
            this->set_error(release_error);
            su_sigabrt();
        }
        if (release_error != ER_SUCCESS)
        {
            this->set_error(release_error);
            return ;
        }
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", previous_pointer);
    }
    this->_pointer = memory_pointer;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
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
