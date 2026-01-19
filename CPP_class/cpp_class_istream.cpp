#include "class_istream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

thread_local ft_operation_error_stack ft_istream::_operation_errors = {{}, {}, 0};

ft_istream::ft_istream() noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_istream::ft_istream(const ft_istream &other) noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    return ;
}

ft_istream::ft_istream(ft_istream &&other) noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._gcount = 0;
    other._bad = false;
    other._error_code = FT_ERR_SUCCESSS;
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_istream::~ft_istream() noexcept
{
    return ;
}

void ft_istream::record_operation_error(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(ft_istream::_operation_errors,
        error_code, operation_id);
    return ;
}

void ft_istream::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_istream::record_operation_error(error_code);
    return ;
}

void ft_istream::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_istream::lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_recursive_mutex> local_guard(this->_mutex);

    if (local_guard.get_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_recursive_mutex>();
        return (local_guard.get_error());
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

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            return (single_guard.get_error());
        }
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

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_recursive_mutex> upper_guard(ordered_second->_mutex);
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

ft_istream &ft_istream::operator=(const ft_istream &other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    return (*this);
}

ft_istream &ft_istream::operator=(ft_istream &&other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._gcount = 0;
    other._bad = false;
    other._error_code = FT_ERR_SUCCESSS;
    other.set_error_unlocked(FT_ERR_SUCCESSS);
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
        this->set_error(lock_error);
        this->_bad = true;
        this->_gcount = 0;
        return ;
    }
    this->_error_code = FT_ERR_SUCCESSS;
    this->_gcount = 0;
    this->_bad = false;
    if (buffer == ft_nullptr && count > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->_bad = true;
        return ;
    }
    bytes_read = this->do_read(buffer, count);
    this->_gcount = bytes_read;
    if (this->_error_code != FT_ERR_SUCCESSS)
        this->_bad = true;
    else
        this->set_error(FT_ERR_SUCCESSS);
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
        this->set_error(lock_error);
        return (0);
    }
    count_value = this->_gcount;
    this->set_error(FT_ERR_SUCCESSS);
    return (count_value);
}

bool ft_istream::bad() const noexcept
{
    int lock_error;
    bool is_bad_result;

    ft_unique_lock<pt_recursive_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (true);
    }
    is_bad_result = this->_bad;
    this->set_error(FT_ERR_SUCCESSS);
    return (is_bad_result);
}

int ft_istream::get_error() const noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    return (this->_error_code);
}

const char *ft_istream::get_error_str() const noexcept
{
    const char *error_string;

    error_string = ft_strerror(this->get_error());
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_istream::last_operation_error_str() noexcept
{
    const char *error_string;

    error_string = ft_strerror(ft_istream::last_operation_error());
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_istream::operation_error_str_at(ft_size_t index) noexcept
{
    const char *error_string;

    error_string = ft_strerror(ft_istream::operation_error_at(index));
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

int ft_istream::last_operation_error() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    return (ft_operation_error_stack_last_error(ft_istream::_operation_errors));
}

int ft_istream::operation_error_at(ft_size_t index) noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    return (ft_operation_error_stack_error_at(ft_istream::_operation_errors, index));
}

void ft_istream::pop_operation_errors() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_operation_error_stack_pop_last(ft_istream::_operation_errors);
    return ;
}

int ft_istream::pop_oldest_operation_error() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    return (ft_operation_error_stack_pop_last(ft_istream::_operation_errors));
}

int ft_istream::operation_error_index() noexcept
{
    ft_size_t index;

    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    index = 0;
    while (index < ft_istream::_operation_errors.count)
    {
        if (ft_istream::_operation_errors.errors[index] != FT_ERR_SUCCESSS)
            return (static_cast<int>(index + 1));
        index++;
    }
    return (0);
}

pt_recursive_mutex *ft_istream::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_istream::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
