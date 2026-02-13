#ifndef FT_STRING_VIEW_HPP
#define FT_STRING_VIEW_HPP

#include <cstddef>
#include <stdint.h>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"

template <typename CharType>
class ft_string_view
{
    private:
        const CharType*         _data;
        size_t                  _size;
        mutable pt_recursive_mutex* _mutex;

        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();
        static int lock_pair(const ft_string_view &first, const ft_string_view &second,
            bool *first_acquired, bool *second_acquired) noexcept;

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

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename CharType>
const size_t ft_string_view<CharType>::npos = static_cast<size_t>(-1);

template <typename CharType>
ft_string_view<CharType>::ft_string_view()
    : _data(ft_nullptr), _size(0), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const CharType* string)
    : _data(string), _size(0), _mutex(ft_nullptr)
{
    if (string == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return ;
    }
    size_t index = 0;

    while (string[index] != CharType())
        ++index;
    this->_size = index;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const CharType* string, size_t size)
    : _data(string), _size(size), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
ft_string_view<CharType>::ft_string_view(const ft_string_view& other)
    : _data(ft_nullptr), _size(0), _mutex(ft_nullptr)
{
    bool lock_acquired = false;
    int lock_result = other.lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_data = other._data;
    this->_size = other._size;
    other.unlock_internal(lock_acquired);
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
ft_string_view<CharType>& ft_string_view<CharType>::operator=(const ft_string_view& other)
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool first_lock_acquired = false;
    bool second_lock_acquired = false;
    int lock_result = ft_string_view<CharType>::lock_pair(*this, other,
            &first_lock_acquired, &second_lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    this->_data = other._data;
    this->_size = other._size;
    if (second_lock_acquired)
        other.unlock_internal(second_lock_acquired);
    if (first_lock_acquired)
        this->unlock_internal(first_lock_acquired);
    this->teardown_thread_safety();
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename CharType>
ft_string_view<CharType>::~ft_string_view()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
const CharType* ft_string_view<CharType>::data() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (ft_nullptr);
    }
    const CharType* result = this->_data;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

template <typename CharType>
size_t ft_string_view<CharType>::size() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    size_t current_size = this->_size;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (current_size);
}

template <typename CharType>
bool ft_string_view<CharType>::empty() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (true);
    }
    bool result = (this->_size == 0);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

template <typename CharType>
CharType ft_string_view<CharType>::operator[](size_t index) const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (CharType());
    }
    if (index >= this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (CharType());
    }
    CharType value = this->_data[index];
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (value);
}

template <typename CharType>
int ft_string_view<CharType>::compare(const ft_string_view& other) const
{
    bool first_acquired = false;
    bool second_acquired = false;
    int lock_result = ft_string_view<CharType>::lock_pair(*this, other,
            &first_acquired, &second_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    const CharType *first_data = this->_data;
    const CharType *second_data = other._data;
    size_t first_size = this->_size;
    size_t second_size = other._size;
    size_t index = 0;
    int result = 0;

    while (index < first_size && index < second_size)
    {
        if (first_data[index] != second_data[index])
        {
            result = (first_data[index] < second_data[index]) ? -1 : 1;
            break;
        }
        ++index;
    }
    if (result == 0)
    {
        if (first_size < second_size)
            result = -1;
        else if (first_size > second_size)
            result = 1;
    }
    if (second_acquired)
        other.unlock_internal(second_acquired);
    if (first_acquired)
        this->unlock_internal(first_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

template <typename CharType>
ft_string_view<CharType> ft_string_view<CharType>::substr(size_t position, size_t count) const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);
    ft_string_view<CharType> result;

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (result);
    }
    if (position > this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    size_t available = this->_size - position;

    if (count == npos || count > available)
        count = available;
    const unsigned char* byte_ptr = reinterpret_cast<const unsigned char*>(this->_data);
    byte_ptr += position * sizeof(CharType);
    const CharType* new_data = reinterpret_cast<const CharType*>(byte_ptr);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (ft_string_view<CharType>(new_data, count));
}

template <typename CharType>
int ft_string_view<CharType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename CharType>
void ft_string_view<CharType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename CharType>
bool ft_string_view<CharType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename CharType>
int ft_string_view<CharType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

template <typename CharType>
void ft_string_view<CharType>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(result);
}

template <typename CharType>
int ft_string_view<CharType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result == FT_ERR_SUCCESS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template <typename CharType>
int ft_string_view<CharType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename CharType>
int ft_string_view<CharType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);
    if (result != FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template <typename CharType>
void ft_string_view<CharType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

template <typename CharType>
int ft_string_view<CharType>::lock_pair(const ft_string_view &first,
    const ft_string_view &second, bool *first_acquired, bool *second_acquired) noexcept
{
    if (first_acquired != ft_nullptr)
        *first_acquired = false;
    if (second_acquired != ft_nullptr)
        *second_acquired = false;
    if (&first == &second)
        return first.lock_internal(first_acquired);
    const ft_string_view *ordered_first = &first;
    const ft_string_view *ordered_second = &second;
    bool swapped = false;

    if (ordered_first > ordered_second)
    {
        const ft_string_view *tmp = ordered_first;
        ordered_first = ordered_second;
        ordered_second = tmp;
        swapped = true;
    }
    bool first_lock = false;
    int first_result = ordered_first->lock_internal(&first_lock);

    if (first_result != FT_ERR_SUCCESS)
        return first_result;
    bool second_lock = false;
    int second_result = ordered_second->lock_internal(&second_lock);

    if (second_result != FT_ERR_SUCCESS)
    {
        ordered_first->unlock_internal(first_lock);
        return second_result;
    }
    if (!swapped)
    {
        if (first_acquired != ft_nullptr)
            *first_acquired = first_lock;
        if (second_acquired != ft_nullptr)
            *second_acquired = second_lock;
    }
    else
    {
        if (first_acquired != ft_nullptr)
            *first_acquired = second_lock;
        if (second_acquired != ft_nullptr)
            *second_acquired = first_lock;
    }
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD

template <typename CharType>
pt_recursive_mutex* ft_string_view<CharType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
