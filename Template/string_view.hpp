#ifndef FT_STRING_VIEW_HPP
#define FT_STRING_VIEW_HPP

#include <cstddef>
#include <stdint.h>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"


template <typename CharType>
class ft_string_view
{
    private:
        const CharType* _data;
        size_t          _size;
        mutable pt_mutex _mutex;
        mutable int     _error_code;

        void set_error(int error) const;

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
    : _data(ft_nullptr), _size(0), _mutex(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const CharType* string)
    : _data(string), _size(0), _mutex(), _error_code(ER_SUCCESS)
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
    : _data(string), _size(size), _mutex(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const ft_string_view& other)
    : _data(ft_nullptr), _size(0), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(other._mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_data = other._data;
    this->_size = other._size;
    this->set_error(other._error_code);
    return ;
}

template <typename CharType>
ft_string_view<CharType>& ft_string_view<CharType>::operator=(const ft_string_view& other)
{
    if (this == &other)
    {
        ft_unique_lock<pt_mutex> self_guard(this->_mutex);

        if (self_guard.get_error() != ER_SUCCESS)
            this->set_error(self_guard.get_error());
        else
            this->set_error(ER_SUCCESS);
        return (*this);
    }
    uintptr_t this_address;
    uintptr_t other_address;
    const ft_string_view<CharType>* first_view;
    const ft_string_view<CharType>* second_view;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_view = this;
        second_view = &other;
    }
    else
    {
        first_view = &other;
        second_view = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first_view->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second_view->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->_data = other._data;
    this->_size = other._size;
    this->set_error(other._error_code);
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
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_data);
}

template <typename CharType>
size_t ft_string_view<CharType>::size() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (this->_size);
}

template <typename CharType>
bool ft_string_view<CharType>::empty() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (true);
    }
    this->set_error(ER_SUCCESS);
    return (this->_size == 0);
}

template <typename CharType>
CharType ft_string_view<CharType>::operator[](size_t index) const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (CharType());
    }
    if (index >= this->_size)
    {
        this->set_error(FT_EINVAL);
        return (CharType());
    }
    this->set_error(ER_SUCCESS);
    return (this->_data[index]);
}

template <typename CharType>
int ft_string_view<CharType>::compare(const ft_string_view& other) const
{
    uintptr_t this_address;
    uintptr_t other_address;
    const ft_string_view<CharType>* first_view;
    const ft_string_view<CharType>* second_view;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_view = this;
        second_view = &other;
    }
    else
    {
        first_view = &other;
        second_view = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first_view->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (0);
    }
    ft_unique_lock<pt_mutex> second_guard(second_view->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (0);
    }
    size_t index;

    index = 0;
    while (index < this->_size && index < other._size)
    {
        if (this->_data[index] != other._data[index])
        {
            if (this->_data[index] < other._data[index])
            {
                this->set_error(ER_SUCCESS);
                return (-1);
            }
            this->set_error(ER_SUCCESS);
            return (1);
        }
        index++;
    }
    if (this->_size == other._size)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    if (this->_size < other._size)
    {
        this->set_error(ER_SUCCESS);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename CharType>
ft_string_view<CharType> ft_string_view<CharType>::substr(size_t position, size_t count) const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ft_string_view<CharType>());
    }
    if (position > this->_size)
    {
        ft_string_view<CharType> result;

        this->set_error(FT_EINVAL);
        result.set_error(FT_EINVAL);
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
    this->set_error(ER_SUCCESS);
    return (ft_string_view<CharType>(new_data, count));
}

template <typename CharType>
void ft_string_view<CharType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename CharType>
int ft_string_view<CharType>::get_error() const
{
    return (this->_error_code);
}

template <typename CharType>
const char* ft_string_view<CharType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif
