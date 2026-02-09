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
    , _initialized(false)
{
    return ;
}

int ft_string::initialize() noexcept
{
    if (this->_initialized)
        return (FT_ERR_SUCCESSS);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->_initialized = true;
    return (FT_ERR_SUCCESSS);
}

int ft_string::initialize(const char *initial_string) noexcept
{
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESSS)
        return (initialization_error);
    if (initial_string)
        this->assign(initial_string, ft_strlen_size_t(initial_string));
    return (FT_ERR_SUCCESSS);
}

int ft_string::initialize(size_t count, char character) noexcept
{
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESSS)
        return (initialization_error);
    this->assign(count, character);
    return (FT_ERR_SUCCESSS);
}

int ft_string::initialize(const ft_string &other) noexcept
{
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESSS)
        return (initialization_error);
    int other_lock_error = FT_ERR_SUCCESSS;
    if (other._mutex != ft_nullptr)
    {
        other_lock_error = other._mutex->lock();
        if (other_lock_error != FT_ERR_SUCCESSS)
        {
            (void)this->destroy();
            return (other_lock_error);
        }
    }
    this->_length = other._length;
    this->_capacity = other._capacity;
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            if (other._mutex != ft_nullptr)
                other._mutex->unlock();
            this->_length = 0;
            this->_capacity = 0;
            (void)this->destroy();
            return (FT_ERR_SYSTEM);
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    else
    {
        this->_data = ft_nullptr;
        this->_capacity = 0;
    }
    if (other._mutex != ft_nullptr)
        other._mutex->unlock();
    return (FT_ERR_SUCCESSS);
}

int ft_string::initialize(ft_string &&other) noexcept
{
    int initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESSS)
        return (initialization_error);
    int move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESSS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESSS);
}

int ft_string::destroy() noexcept
{
    if (!this->_initialized)
        return (FT_ERR_SUCCESSS);
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    (void)this->disable_thread_safety();
    this->_initialized = false;
    return (FT_ERR_SUCCESSS);
}

ft_string::~ft_string()
{
    (void)this->destroy();
    return ;
}
