#include "class_istream.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "class_nullptr.hpp"
#include <new>

int ft_istream::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_istream::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_istream::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    int mutex_error;

    if (mutex_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_istream::teardown_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int ft_istream::enable_thread_safety(void) noexcept
{
    return (this->prepare_thread_safety());
}

void ft_istream::disable_thread_safety(void) noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_istream::is_thread_safe_enabled(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_istream::lock_pair(const ft_istream &first, const ft_istream &second,
        const ft_istream *&lower, const ft_istream *&upper) noexcept
{
    const ft_istream *ordered_first = &first;
    const ft_istream *ordered_second = &second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    if (ordered_first > ordered_second)
    {
        const ft_istream *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = lower->lock_mutex();

        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int upper_error = upper->lock_mutex();

        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
        pt_thread_sleep(1);
    }
}

int ft_istream::unlock_pair(const ft_istream *lower, const ft_istream *upper) noexcept
{
    int error;
    int final_error = FT_ERR_SUCCESS;

    if (upper != ft_nullptr)
    {
        error = upper->unlock_mutex();
        if (error != FT_ERR_SUCCESS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = lower->unlock_mutex();
        if (error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
            final_error = error;
    }
    return (final_error);
}

ft_istream::ft_istream() noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_istream::ft_istream(const ft_istream &other) noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    const ft_istream *lower = ft_nullptr;
    const ft_istream *upper = ft_nullptr;
    int lock_error = ft_istream::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    int unlock_error = ft_istream::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_istream::ft_istream(ft_istream &&other) noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    const ft_istream *lower = ft_nullptr;
    const ft_istream *upper = ft_nullptr;
    int lock_error = ft_istream::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    other._gcount = 0;
    other._is_valid = true;
    int unlock_error = ft_istream::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_istream::~ft_istream() noexcept
{
    this->teardown_thread_safety();
    return ;
}

ft_istream &ft_istream::operator=(const ft_istream &other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    const ft_istream *lower = ft_nullptr;
    const ft_istream *upper = ft_nullptr;
    int lock_error = ft_istream::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    int unlock_error = ft_istream::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(unlock_error);
        return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

ft_istream &ft_istream::operator=(ft_istream &&other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    const ft_istream *lower = ft_nullptr;
    const ft_istream *upper = ft_nullptr;
    int lock_error = ft_istream::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    other._gcount = 0;
    other._is_valid = true;
    int unlock_error = ft_istream::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(unlock_error);
        return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

void ft_istream::read(char *buffer, std::size_t count)
{
    std::size_t bytes_read;
    int lock_error = this->lock_mutex();
    int operation_error = FT_ERR_SUCCESS;

    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        this->_gcount = 0;
        return ;
    }
    this->_gcount = 0;
    this->_is_valid = true;
    if (buffer == ft_nullptr && count > 0)
    {
        int unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESS)
            ft_global_error_stack_push(unlock_error);
        else
            ft_global_error_stack_push(FT_ERR_SUCCESS);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        this->_is_valid = false;
        return ;
    }
    bytes_read = this->do_read(buffer, count);
    this->_gcount = bytes_read;
    int last_error = ft_global_error_stack_peek_last_error();
    this->_is_valid = (last_error == FT_ERR_SUCCESS);
    operation_error = last_error;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    ft_global_error_stack_push(operation_error);
    return ;
}

std::size_t ft_istream::gcount() const noexcept
{
    std::size_t count_value;
    int lock_error = this->lock_mutex();

    if (lock_error != FT_ERR_SUCCESS)
    {
        return (0);
    }
    count_value = this->_gcount;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (count_value);
}

bool ft_istream::is_valid() const noexcept
{
    bool is_valid_result;
    int lock_error = this->lock_mutex();

    if (lock_error != FT_ERR_SUCCESS)
    {
        return (false);
    }
    is_valid_result = this->_is_valid;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (is_valid_result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_istream::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}

#endif
