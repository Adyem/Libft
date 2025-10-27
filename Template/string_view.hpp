#ifndef FT_STRING_VIEW_HPP
#define FT_STRING_VIEW_HPP

#include <cstddef>
#include <stdint.h>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/pthread.hpp"
#include "move.hpp"

template <typename CharType>
class ft_string_view
{
    private:
        const CharType*         _data;
        size_t                  _size;
        mutable int             _error_code;
        mutable pt_mutex        _mutex;

        void set_error(int error) const;
        static void sleep_backoff() noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard,
            int entry_errno) noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static int lock_pair(const ft_string_view &first,
            const ft_string_view &second,
            ft_unique_lock<pt_mutex> &first_guard,
            ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
        static const size_t npos;

        ft_string_view();
        ft_string_view(const CharType* string);
        ft_string_view(const CharType* string, size_t size);
        ft_string_view(const ft_string_view& other);
        ft_string_view& operator=(const ft_string_view& other);
        ~ft_string_view();

        const CharType* data() const;
        size_t size() const;
        bool empty() const;
        CharType operator[](size_t index) const;

        int compare(const ft_string_view& other) const;
        ft_string_view substr(size_t position, size_t count = npos) const;

        int get_error() const;
        const char* get_error_str() const;
};

template <typename CharType>
const size_t ft_string_view<CharType>::npos = static_cast<size_t>(-1);

template <typename CharType>
ft_string_view<CharType>::ft_string_view()
    : _data(ft_nullptr), _size(0), _error_code(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const CharType* string)
    : _data(string), _size(0), _error_code(ER_SUCCESS), _mutex()
{
    if (string == ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    size_t index;
    index = 0;
    while (string[index] != CharType())
    {
        index++;
    }
    _size = index;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const CharType* string, size_t size)
    : _data(string), _size(size), _error_code(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const ft_string_view& other)
    : _data(ft_nullptr), _size(0), _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->_data = ft_nullptr;
        this->_size = 0;
        this->_error_code = lock_error;
        this->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_data = other._data;
    this->_size = other._size;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    ft_string_view<CharType>::restore_errno(other_guard, entry_errno);
    return ;
}

template <typename CharType>
ft_string_view<CharType>& ft_string_view<CharType>::operator=(const ft_string_view& other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    entry_errno = ft_errno;
    lock_error = ft_string_view<CharType>::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(this_guard, entry_errno);
        ft_string_view<CharType>::restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_data = other._data;
    this->_size = other._size;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    ft_string_view<CharType>::restore_errno(other_guard, entry_errno);
    ft_string_view<CharType>::restore_errno(this_guard, entry_errno);
    return (*this);
}

template <typename CharType>
ft_string_view<CharType>::~ft_string_view()
{
    return ;
}

template <typename CharType>
const CharType* ft_string_view<CharType>::data() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    const CharType *result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    result = this->_data;
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (result);
}

template <typename CharType>
size_t ft_string_view<CharType>::size() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    size_t size_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (0);
    }
    size_value = this->_size;
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (size_value);
}

template <typename CharType>
bool ft_string_view<CharType>::empty() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    bool result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (true);
    }
    result = (this->_size == 0);
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (result);
}

template <typename CharType>
CharType ft_string_view<CharType>::operator[](size_t index) const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    CharType value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (CharType());
    }
    if (index >= this->_size)
    {
        const_cast<ft_string_view*>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (CharType());
    }
    value = this->_data[index];
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (value);
}

template <typename CharType>
int ft_string_view<CharType>::compare(const ft_string_view& other) const
{
    ft_unique_lock<pt_mutex> first_guard;
    ft_unique_lock<pt_mutex> second_guard;
    int entry_errno;
    int lock_error;
    const CharType *first_data;
    const CharType *second_data;
    size_t first_size;
    size_t second_size;
    size_t index;
    int result;

    entry_errno = ft_errno;
    lock_error = ft_string_view<CharType>::lock_pair(*this, other,
            first_guard, second_guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        const_cast<ft_string_view*>(&other)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(first_guard, entry_errno);
        ft_string_view<CharType>::restore_errno(second_guard, entry_errno);
        return (0);
    }
    first_data = this->_data;
    second_data = other._data;
    first_size = this->_size;
    second_size = other._size;
    index = 0;
    result = 0;
    while (index < first_size && index < second_size)
    {
        if (first_data[index] != second_data[index])
        {
            if (first_data[index] < second_data[index])
                result = -1;
            else
                result = 1;
            break;
        }
        index++;
    }
    if (result == 0)
    {
        if (first_size == second_size)
            result = 0;
        else if (first_size < second_size)
            result = -1;
        else
            result = 1;
    }
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    const_cast<ft_string_view*>(&other)->set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(second_guard, entry_errno);
    ft_string_view<CharType>::restore_errno(first_guard, entry_errno);
    return (result);
}

template <typename CharType>
ft_string_view<CharType> ft_string_view<CharType>::substr(size_t position, size_t count) const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    ft_string_view<CharType> result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error(lock_error);
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (result);
    }
    if (position > this->_size)
    {
        const_cast<ft_string_view*>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        result.set_error(FT_ERR_INVALID_ARGUMENT);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (result);
    }
    size_t available;

    available = this->_size - position;
    if (count == npos || count > available)
        count = available;
    const unsigned char* byte_ptr;

    byte_ptr = reinterpret_cast<const unsigned char*>(this->_data);
    byte_ptr += position * sizeof(CharType);
    const CharType* new_data;

    new_data = reinterpret_cast<const CharType*>(byte_ptr);
    const_cast<ft_string_view*>(this)->set_error(ER_SUCCESS);
    result = ft_string_view<CharType>(new_data, count);
    result.set_error(ER_SUCCESS);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (result);
}

template <typename CharType>
void ft_string_view<CharType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename CharType>
void ft_string_view<CharType>::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

template <typename CharType>
void ft_string_view<CharType>::restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

template <typename CharType>
int ft_string_view<CharType>::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    guard = ft_unique_lock<pt_mutex>(const_cast<pt_mutex&>(this->_mutex));
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (guard.get_error());
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

template <typename CharType>
int ft_string_view<CharType>::lock_pair(const ft_string_view &first,
    const ft_string_view &second, ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_string_view *ordered_first;
    const ft_string_view *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(
            const_cast<pt_mutex&>(first._mutex));

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
        const ft_string_view *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(
            const_cast<pt_mutex&>(ordered_first->_mutex));

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(
            const_cast<pt_mutex&>(ordered_second->_mutex));
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
        ft_string_view<CharType>::sleep_backoff();
    }
}

template <typename CharType>
int ft_string_view<CharType>::get_error() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_code;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_code = this->_error_code;
    const_cast<ft_string_view*>(this)->set_error(error_code);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (error_code);
}

template <typename CharType>
const char* ft_string_view<CharType>::get_error_str() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_code;
    const char *result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        const_cast<ft_string_view*>(this)->set_error(lock_error);
        ft_string_view<CharType>::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_code = this->_error_code;
    result = ft_strerror(error_code);
    const_cast<ft_string_view*>(this)->set_error(error_code);
    ft_string_view<CharType>::restore_errno(guard, entry_errno);
    return (result);
}

#endif
