#include "class_stringbuf.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"
#include <mutex>

thread_local ft_operation_error_stack ft_stringbuf::_operation_errors = {{}, {}, 0};

void ft_stringbuf::record_operation_error(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(ft_stringbuf::_operation_errors,
        error_code, operation_id);
    return ;
}

void ft_stringbuf::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_stringbuf::record_operation_error(error_code);
    return ;
}

void ft_stringbuf::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_stringbuf::lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
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

int ft_stringbuf::lock_pair(const ft_stringbuf &first, const ft_stringbuf &second,
        ft_unique_lock<pt_recursive_mutex> &first_guard,
        ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept
{
    const ft_stringbuf *ordered_first;
    const ft_stringbuf *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_recursive_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
            return (single_guard.get_error());
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_recursive_mutex>();
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_stringbuf *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_recursive_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
            return (lower_guard.get_error());
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
            return (upper_guard.get_error());
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_stringbuf::ft_stringbuf(const ft_string &string) noexcept
    : _storage(string)
    , _position(0)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    int storage_error;

    storage_error = ft_string::last_operation_error();
    if (storage_error != FT_ERR_SUCCESSS)
        this->set_error(storage_error);
    else
        this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_stringbuf::ft_stringbuf(const ft_stringbuf &other) noexcept
    : _storage(other._storage)
    , _position(other._position)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_position = 0;
        this->set_error(lock_error);
        return ;
    }
    this->_error_code = other._error_code;
    int string_error;

    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
        this->set_error(string_error);
    else
        this->set_error(this->_error_code);
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(const ft_stringbuf &other) noexcept
{
    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    ft_unique_lock<pt_recursive_mutex> first_guard;
    ft_unique_lock<pt_recursive_mutex> second_guard;
    int lock_error;

    lock_error = ft_stringbuf::lock_pair(*this, other, first_guard, second_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_storage = other._storage;
    this->_position = other._position;
    this->_error_code = other._error_code;
    int string_error;

    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
        this->set_error(string_error);
    else
        this->set_error(this->_error_code);
    return (*this);
}

ft_stringbuf::ft_stringbuf(ft_stringbuf &&other) noexcept
    : _storage(ft_move(other._storage))
    , _position(other._position)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_position = 0;
        this->set_error(lock_error);
        return ;
    }
    this->_error_code = other._error_code;
    other._position = 0;
    other._error_code = FT_ERR_SUCCESSS;
    int string_error;

    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
        this->set_error(string_error);
    else
        this->set_error(this->_error_code);
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(ft_stringbuf &&other) noexcept
{
    if (this == &other)
        return (*this);
    ft_unique_lock<pt_recursive_mutex> first_guard;
    ft_unique_lock<pt_recursive_mutex> second_guard;
    int lock_error;

    lock_error = ft_stringbuf::lock_pair(*this, other, first_guard, second_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_storage = ft_move(other._storage);
    this->_position = other._position;
    this->_error_code = other._error_code;
    other._position = 0;
    other._error_code = FT_ERR_SUCCESSS;
    int string_error;

    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
        this->set_error(string_error);
    else
        this->set_error(this->_error_code);
    return (*this);
}

ft_stringbuf::~ft_stringbuf() noexcept
{
    return ;
}

std::size_t ft_stringbuf::read(char *buffer, std::size_t count)
{
    int lock_error;
    std::size_t index;
    bool failure_occurred;

    ft_unique_lock<pt_recursive_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (0);
    }
    if (buffer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    index = 0;
    failure_occurred = false;
    while (index < count && this->_position < this->_storage.size())
    {
        const char *current;

        current = this->_storage.at(this->_position);
        if (!current)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            failure_occurred = true;
            break ;
        }
        buffer[index] = *current;
        index++;
        this->_position++;
    }
    if (!failure_occurred)
        this->set_error(FT_ERR_SUCCESSS);
    return (index);
}

bool ft_stringbuf::is_bad() const noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    bool result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (true);
    }
    result = (this->_error_code != FT_ERR_SUCCESSS);
    this->set_error(this->_error_code);
    return (result);
}

int ft_stringbuf::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_stringbuf::get_error_str() const noexcept
{
    const char *error_string;

    error_string = ft_strerror(this->get_error());
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

ft_string ft_stringbuf::str() const
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    const char *start;
    ft_string result;
    int string_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (ft_string(lock_error));
    }
    start = this->_storage.c_str();
    result = ft_string(start + this->_position);
    string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
        this->set_error(string_error);
    else
        this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

const char *ft_stringbuf::last_operation_error_str() noexcept
{
    const char *error_string;

    error_string = ft_strerror(ft_stringbuf::last_operation_error());
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_stringbuf::operation_error_str_at(ft_size_t index) noexcept
{
    const char *error_string;

    error_string = ft_strerror(ft_stringbuf::operation_error_at(index));
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

int ft_stringbuf::last_operation_error() noexcept
{

    return (ft_operation_error_stack_last_error(ft_stringbuf::_operation_errors));
}

int ft_stringbuf::operation_error_at(ft_size_t index) noexcept
{

    return (ft_operation_error_stack_error_at(ft_stringbuf::_operation_errors, index));
}

void ft_stringbuf::pop_operation_errors() noexcept
{

    ft_operation_error_stack_pop_last(ft_stringbuf::_operation_errors);
    return ;
}

int ft_stringbuf::pop_oldest_operation_error() noexcept
{

    return (ft_operation_error_stack_pop_last(ft_stringbuf::_operation_errors));
}

int ft_stringbuf::operation_error_index() noexcept
{
    ft_size_t index;


    index = 0;
    while (index < ft_stringbuf::_operation_errors.count)
    {
        if (ft_stringbuf::_operation_errors.errors[index] != FT_ERR_SUCCESSS)
            return (static_cast<int>(index + 1));
        index++;
    }
    return (0);
}

pt_recursive_mutex *ft_stringbuf::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_stringbuf::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif
