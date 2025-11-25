#include "class_istream.hpp"
#include "class_nullptr.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

ft_istream::ft_istream() noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    return ;
}

ft_istream::ft_istream(const ft_istream &other) noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_istream::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    ft_istream::restore_errno(other_guard, entry_errno);
    return ;
}

ft_istream::ft_istream(ft_istream &&other) noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_istream::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._gcount = 0;
    other._bad = false;
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    ft_istream::restore_errno(other_guard, entry_errno);
    return ;
}

ft_istream::~ft_istream() noexcept
{
    return ;
}

void ft_istream::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_istream::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_istream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

void ft_istream::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno, bool restore_previous_on_success) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != ER_SUCCESS)
    {
        ft_errno = operation_errno;
        return ;
    }
    if (restore_previous_on_success)
    {
        ft_errno = entry_errno;
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

int ft_istream::lock_pair(const ft_istream &first, const ft_istream &second,
    ft_unique_lock<pt_mutex> &first_guard, ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_istream *ordered_first;
    const ft_istream *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
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
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_istream &ft_istream::operator=(const ft_istream &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_gcount = other._gcount;
    this->_bad = other._bad;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    ft_istream::restore_errno(this_guard, entry_errno);
    ft_istream::restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_istream &ft_istream::operator=(ft_istream &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_istream::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
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
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    ft_istream::restore_errno(this_guard, entry_errno);
    ft_istream::restore_errno(other_guard, entry_errno);
    return (*this);
}

void ft_istream::read(char *buffer, std::size_t count)
{
    int entry_errno;
    int lock_error;
    std::size_t readed;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard;
    ft_errno = entry_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        this->_bad = true;
        this->_gcount = 0;
        ft_istream::restore_errno(guard, entry_errno);
        return ;
    }
    this->_error_code = ER_SUCCESS;
    this->_gcount = 0;
    this->_bad = false;
    if (buffer == ft_nullptr && count > 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->_bad = true;
        ft_istream::restore_errno(guard, entry_errno);
        return ;
    }
    readed = this->do_read(buffer, count);
    this->_gcount = readed;
    if (this->_error_code != ER_SUCCESS)
        this->_bad = true;
    ft_istream::restore_errno(guard, entry_errno, false);
    return ;
}

std::size_t ft_istream::gcount() const noexcept
{
    int entry_errno;
    int lock_error;
    std::size_t count_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard;
    ft_errno = entry_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (0);
    }
    count_value = this->_gcount;
    ft_istream::restore_errno(guard, entry_errno);
    return (count_value);
}

bool ft_istream::bad() const noexcept
{
    int entry_errno;
    int lock_error;
    bool is_bad_result;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard;
    ft_errno = entry_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (true);
    }
    is_bad_result = this->_bad;
    ft_istream::restore_errno(guard, entry_errno);
    return (is_bad_result);
}

int ft_istream::get_error() const noexcept
{
    int entry_errno;
    int lock_error;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard;
    ft_errno = entry_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_code = this->_error_code;
    ft_istream::restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_istream::get_error_str() const noexcept
{
    int entry_errno;
    int lock_error;
    const char *error_string;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard;
    ft_errno = entry_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    ft_istream::restore_errno(guard, entry_errno);
    return (error_string);
}
