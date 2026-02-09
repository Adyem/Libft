#ifndef FT_SET_HPP
#define FT_SET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"

template <typename ElementType>
class ft_set
{
    private:
        ElementType*                   _data;
        size_t                         _capacity;
        size_t                         _size;
        mutable pt_recursive_mutex*    _mutex;

        bool    ensure_capacity(size_t desired_capacity);
        size_t  find_index(const ElementType& value) const;
        size_t  lower_bound(const ElementType& value) const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        ft_set(size_t initial_capacity = 0);
        ~ft_set();

        ft_set(const ft_set&) = delete;
        ft_set& operator=(const ft_set&) = delete;

        ft_set(ft_set&& other) noexcept;
        ft_set& operator=(ft_set&& other) noexcept;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        void insert(const ElementType& value);
        void insert(ElementType&& value);
        ElementType* find(const ElementType& value);
        const ElementType* find(const ElementType& value) const;
        void remove(const ElementType& value);
        size_t size() const;
        bool empty() const;
        void clear();

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
    #endif
};

template <typename ElementType>
ft_set<ElementType>::ft_set(size_t initial_capacity)
    : _data(ft_nullptr), _capacity(0), _size(0), _mutex(ft_nullptr)
{
    if (initial_capacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initial_capacity));
        if (this->_data == ft_nullptr)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = initial_capacity;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_set<ElementType>::~ft_set()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_set<ElementType>::ft_set(ft_set&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size),
      _mutex(other._mutex)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._mutex = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_set<ElementType>& ft_set<ElementType>::operator=(ft_set&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    if (this->_data != ft_nullptr)
    {
        size_t index = 0;

        while (index < this->_size)
        {
            destroy_at(&this->_data[index]);
            ++index;
        }
        cma_free(this->_data);
    }
    this->teardown_thread_safety();
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    this->_mutex = other._mutex;
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._mutex = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ElementType>
int ft_set<ElementType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_set<ElementType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_set<ElementType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(const ElementType& value)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    size_t position = this->lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t index = this->_size;

    while (index > position)
    {
        construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], value);
    ++this->_size;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(ElementType&& value)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    size_t position = this->lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t index = this->_size;

    while (index > position)
    {
        construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], ft_move(value));
    ++this->_size;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
ElementType* ft_set<ElementType>::find(const ElementType& value)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (ft_nullptr);
    }
    size_t index = this->find_index(value);
    if (index == this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        return (ft_nullptr);
    }
    ElementType *result = &this->_data[index];

    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
const ElementType* ft_set<ElementType>::find(const ElementType& value) const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (ft_nullptr);
    }
    size_t index = this->find_index(value);
    if (index == this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        return (ft_nullptr);
    }
    const ElementType *result = &this->_data[index];

    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::remove(const ElementType& value)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    size_t index = this->find_index(value);
    if (index == this->_size)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        return ;
    }
    destroy_at(&this->_data[index]);
    size_t current_index = index;

    while (current_index + 1 < this->_size)
    {
        construct_at(&this->_data[current_index], ft_move(this->_data[current_index + 1]));
        destroy_at(&this->_data[current_index + 1]);
        ++current_index;
    }
    --this->_size;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
size_t ft_set<ElementType>::size() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    size_t current_size = this->_size;

    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (current_size);
}

template <typename ElementType>
bool ft_set<ElementType>::empty() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (true);
    }
    bool result = (this->_size == 0);

    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::clear()
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    size_t index = 0;

    while (index < this->_size)
    {
        destroy_at(&this->_data[index]);
        ++index;
    }
    this->_size = 0;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex *ft_set<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template <typename ElementType>
bool ft_set<ElementType>::ensure_capacity(size_t desired_capacity)
{
    if (desired_capacity <= this->_capacity)
        return (true);
    size_t new_capacity = (this->_capacity == 0) ? 1 : this->_capacity * 2;

    while (new_capacity < desired_capacity)
        new_capacity *= 2;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));

    if (new_data == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t index = 0;

    while (index < this->_size)
    {
        construct_at(&new_data[index], ft_move(this->_data[index]));
        destroy_at(&this->_data[index]);
        ++index;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return (true);
}

template <typename ElementType>
size_t ft_set<ElementType>::find_index(const ElementType& value) const
{
    size_t left = 0;
    size_t right = this->_size;

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;

        if (this->_data[mid] < value)
            left = mid + 1;
        else if (value < this->_data[mid])
            right = mid;
        else
            return (mid);
    }
    return (this->_size);
}

template <typename ElementType>
size_t ft_set<ElementType>::lower_bound(const ElementType& value) const
{
    size_t left = 0;
    size_t right = this->_size;

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;

        if (this->_data[mid] < value)
            left = mid + 1;
        else
            right = mid;
    }
    return (left);
}

template <typename ElementType>
int ft_set<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename ElementType>
int ft_set<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType>
int ft_set<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);
    if (result != FT_ERR_SUCCESSS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#endif
