#include "class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "class_nullptr.hpp"

ft_string::ft_string() noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_string::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    return ;
}

ft_string::ft_string(const ft_string &other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_string::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    this->_operation_error = this->initialize(other);
    if (this->_operation_error == FT_ERR_SUCCESS)
        this->_operation_error = other._operation_error;
    ft_string::set_last_operation_error(this->_operation_error);
    return ;
}

ft_string::ft_string(ft_string &&other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_string::_state_uninitialized)
    , _operation_error(FT_ERR_SUCCESS)
{
    this->_operation_error = this->initialize(static_cast<ft_string &&>(other));
    if (this->_operation_error == FT_ERR_SUCCESS)
    {
        this->_operation_error = other._operation_error;
        other._operation_error = FT_ERR_SUCCESS;
    }
    ft_string::set_last_operation_error(this->_operation_error);
    return ;
}

ft_string::ft_string(int32_t error_code) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_string::_state_uninitialized)
    , _operation_error(error_code)
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        this->_operation_error = initialization_error;
        ft_string::set_last_operation_error(initialization_error);
    }
    else
    {
        this->_operation_error = error_code;
        ft_string::set_last_operation_error(error_code);
    }
    return ;
}

int ft_string::initialize() noexcept
{
    if (this->_initialized_state == ft_string::_state_initialized)
    {
        this->abort_lifecycle_error("ft_string::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = ft_string::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_string::initialize(const char *initial_string) noexcept
{
    int initialization_error = this->initialize();
    int assign_error;

    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    if (initial_string)
    {
        assign_error = this->assign(initial_string, ft_strlen_size_t(initial_string));
        if (assign_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            return (assign_error);
        }
    }
    return (FT_ERR_SUCCESS);
}

int ft_string::initialize(size_t count, char character) noexcept
{
    int initialization_error = this->initialize();
    int assign_error;

    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    assign_error = this->assign(count, character);
    if (assign_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (assign_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_string::initialize(const ft_string &other) noexcept
{
    if (other._initialized_state == ft_string::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_string::initialize(const ft_string &) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_string::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this == &other)
        return (FT_ERR_SUCCESS);
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    int other_lock_error = FT_ERR_SUCCESS;
    if (other._mutex != ft_nullptr)
    {
        other_lock_error = other._mutex->lock();
        if (other_lock_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            return (other_lock_error);
        }
    }
    this->_length = other._length;
    this->_capacity = other._capacity;
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_malloc(this->_capacity + 1));
        if (!this->_data)
        {
            if (other._mutex != ft_nullptr)
                other._mutex->unlock();
            this->_length = 0;
            this->_capacity = 0;
            (void)this->destroy();
            return (FT_ERR_NO_MEMORY);
        }
        ft_memset(this->_data, 0, this->_capacity + 1);
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    else
    {
        this->_data = ft_nullptr;
        this->_capacity = 0;
    }
    if (other._mutex != ft_nullptr)
        other._mutex->unlock();
    return (FT_ERR_SUCCESS);
}

int ft_string::initialize(ft_string &&other) noexcept
{
    if (other._initialized_state == ft_string::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_string::initialize(ft_string &&) source",
            "called with uninitialized source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialized_state != ft_string::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this == &other)
        return (FT_ERR_SUCCESS);
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    int move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_string::destroy() noexcept
{
    if (this->_initialized_state != ft_string::_state_initialized)
    {
        this->_initialized_state = ft_string::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    (void)this->disable_thread_safety();
    this->_initialized_state = ft_string::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

ft_string::~ft_string()
{
    if (this->_initialized_state != ft_string::_state_initialized)
    {
        this->_initialized_state = ft_string::_state_destroyed;
        return ;
    }
    (void)this->destroy();
    return ;
}

ft_string &ft_string::operator=(const ft_string &other) noexcept
{
    int32_t assignment_error;

    if (this == &other)
    {
        ft_string::set_last_operation_error(FT_ERR_SUCCESS);
        return (*this);
    }
    if (other._initialized_state == ft_string::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_string::operator=(const ft_string &) source",
            "called with uninitialized source object");
        ft_string::set_last_operation_error(FT_ERR_INVALID_STATE);
        return (*this);
    }
    if (other._initialized_state != ft_string::_state_initialized)
    {
        ft_string::set_last_operation_error(FT_ERR_INVALID_STATE);
        return (*this);
    }
    if (this->_initialized_state == ft_string::_state_initialized)
    {
        assignment_error = this->destroy();
        if (assignment_error != FT_ERR_SUCCESS)
        {
            ft_string::set_last_operation_error(assignment_error);
            return (*this);
        }
    }
    assignment_error = this->initialize(other);
    if (assignment_error == FT_ERR_SUCCESS)
        this->_operation_error = other._operation_error;
    else
        this->_operation_error = assignment_error;
    ft_string::set_last_operation_error(this->_operation_error);
    return (*this);
}

ft_string &ft_string::operator=(ft_string &&other) noexcept
{
    int32_t assignment_error;

    if (other._initialized_state == ft_string::_state_uninitialized)
    {
        other.abort_lifecycle_error("ft_string::operator=(ft_string &&) source",
            "called with uninitialized source object");
        ft_string::set_last_operation_error(FT_ERR_INVALID_STATE);
        return (*this);
    }
    if (other._initialized_state != ft_string::_state_initialized)
    {
        ft_string::set_last_operation_error(FT_ERR_INVALID_STATE);
        return (*this);
    }
    if (this == &other)
    {
        ft_string::set_last_operation_error(FT_ERR_SUCCESS);
        return (*this);
    }
    if (this->_initialized_state == ft_string::_state_initialized)
    {
        assignment_error = this->destroy();
        if (assignment_error != FT_ERR_SUCCESS)
        {
            ft_string::set_last_operation_error(assignment_error);
            return (*this);
        }
    }
    assignment_error = this->initialize(static_cast<ft_string &&>(other));
    if (assignment_error == FT_ERR_SUCCESS)
    {
        this->_operation_error = other._operation_error;
        other._operation_error = FT_ERR_SUCCESS;
    }
    else
        this->_operation_error = assignment_error;
    ft_string::set_last_operation_error(this->_operation_error);
    return (*this);
}

ft_string &ft_string::operator=(const char *string) noexcept
{
    int32_t assignment_error;

    if (this->_initialized_state != ft_string::_state_initialized)
    {
        assignment_error = this->initialize();
        if (assignment_error != FT_ERR_SUCCESS)
        {
            ft_string::set_last_operation_error(assignment_error);
            return (*this);
        }
    }
    if (string == ft_nullptr)
        assignment_error = this->clear();
    else
        assignment_error = this->assign(string, ft_strlen_size_t(string));
    this->_operation_error = assignment_error;
    ft_string::set_last_operation_error(this->_operation_error);
    return (*this);
}

ft_string &ft_string::operator=(char character) noexcept
{
    char buffer[2];

    buffer[0] = character;
    buffer[1] = '\0';
    return ((*this) = buffer);
}
