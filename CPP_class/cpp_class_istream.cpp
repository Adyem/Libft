#include "class_istream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

ft_istream::ft_istream() noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex()
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_istream::ft_istream(const ft_istream &other) noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_istream::ft_istream(ft_istream &&other) noexcept
    : _gcount(0)
    , _is_valid(true)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    other._gcount = 0;
    other._is_valid = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_istream::~ft_istream() noexcept
{
    return ;
}

int ft_istream::lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_recursive_mutex> local_guard(this->_mutex);

    if (local_guard.last_operation_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_recursive_mutex>();
        return (local_guard.last_operation_error());
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

int ft_istream::lock_pair(const ft_istream &first, const ft_istream &second,
    ft_unique_lock<pt_recursive_mutex> &first_guard,
    ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept
{
    const ft_istream *ordered_first;
    const ft_istream *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_recursive_mutex> single_guard(first._mutex);

        if (single_guard.last_operation_error() != FT_ERR_SUCCESSS)
            return (single_guard.last_operation_error());
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_recursive_mutex>();
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_istream *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_recursive_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.last_operation_error() != FT_ERR_SUCCESSS)
            return (lower_guard.last_operation_error());
        ft_unique_lock<pt_recursive_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.last_operation_error() == FT_ERR_SUCCESSS)
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
        if (upper_guard.last_operation_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
            return (upper_guard.last_operation_error());
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_istream &ft_istream::operator=(const ft_istream &other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

ft_istream &ft_istream::operator=(ft_istream &&other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_is_valid = false;
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_is_valid = other._is_valid;
    other._gcount = 0;
    other._is_valid = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

void ft_istream::read(char *buffer, std::size_t count)
{
    int lock_error;
    std::size_t bytes_read;

    ft_unique_lock<pt_recursive_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        this->_is_valid = false;
        this->_gcount = 0;
        return ;
    }
    this->_gcount = 0;
    this->_is_valid = true;
    if (buffer == ft_nullptr && count > 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        this->_is_valid = false;
        return ;
    }
    bytes_read = this->do_read(buffer, count);
    this->_gcount = bytes_read;
    int last_error = ft_global_error_stack_last_error();
    this->_is_valid = (last_error == FT_ERR_SUCCESSS);
    return ;
}

std::size_t ft_istream::gcount() const noexcept
{
    int lock_error;
    std::size_t count_value;

    ft_unique_lock<pt_recursive_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    count_value = this->_gcount;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (count_value);
}

bool ft_istream::is_valid() const noexcept
{
    int lock_error;
    bool is_valid_result;

    ft_unique_lock<pt_recursive_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    is_valid_result = this->_is_valid;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (is_valid_result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_istream::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

#endif
