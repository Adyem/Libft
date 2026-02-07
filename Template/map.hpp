#ifndef FT_MAP_HPP
#define FT_MAP_HPP

#include "pair.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include "swap.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>

template <typename Key, typename MappedType>
class ft_map
{
    private:
        Pair<Key, MappedType>*      _data;
        size_t                      _capacity;
        size_t                      _size;
        mutable pt_recursive_mutex* _mutex;

        bool    ensure_capacity(size_t desired_capacity);
        size_t  find_index(const Key& key) const;
        void    destroy_all_unlocked();
        Pair<Key, MappedType>*  end_pointer_unlocked();
        const Pair<Key, MappedType>* end_pointer_unlocked() const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        ft_map(size_t initial_capacity = 10);
        ft_map(const ft_map& other);
        ft_map(ft_map&& other) noexcept;
        ft_map& operator=(const ft_map& other);
        ft_map& operator=(ft_map&& other) noexcept;
        ~ft_map();

        void        insert(const Key& key, const MappedType& value);
        void        insert(const Key& key, MappedType&& value);
        Pair<Key, MappedType>* find(const Key& key);
        const Pair<Key, MappedType>* find(const Key& key) const;
        void        remove(const Key& key);
        bool        empty() const;
        void        clear();
        size_t      size() const;
        size_t      capacity() const;
        Pair<Key, MappedType>* end();
        const Pair<Key, MappedType>* end() const;
        MappedType& at(const Key& key);
        const MappedType& at(const Key& key) const;

        int         enable_thread_safety();
        void        disable_thread_safety();
        bool        is_thread_safe_enabled() const;
        int         lock(bool *lock_acquired) const;
        void        unlock(bool lock_acquired) const;

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
    #endif
};

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(size_t initial_capacity)
    : _data(ft_nullptr), _capacity(initial_capacity), _size(0), _mutex(ft_nullptr)
{
    if (this->_capacity > 0)
    {
        void *raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);

        if (raw_memory == ft_nullptr)
        {
            this->_capacity = 0;
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(const ft_map<Key, MappedType>& other)
    : _data(ft_nullptr), _capacity(other._capacity), _size(other._size), _mutex(ft_nullptr)
{
    bool    lock_acquired = false;
    int     lock_error = other.lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_capacity > 0)
    {
        void *raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);

        if (raw_memory == ft_nullptr)
        {
            this->_capacity = 0;
            this->_size = 0;
            other.unlock_internal(lock_acquired);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
        size_t index = 0;

        while (index < this->_size)
        {
            construct_at(&this->_data[index], other._data[index]);
            index++;
        }
    }
    other.unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(ft_map<Key, MappedType>&& other) noexcept
    : _data(ft_nullptr), _capacity(0), _size(0), _mutex(ft_nullptr)
{
    bool    lock_acquired = false;
    bool    other_thread_safe = false;
    int     lock_error = other.lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    other_thread_safe = (other._mutex != ft_nullptr);
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
        {
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(const ft_map<Key, MappedType>& other)
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    ft_map copy(other);

    if (copy._data == ft_nullptr && other._capacity > 0)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (*this);
    }
    ft_swap(this->_data, copy._data);
    ft_swap(this->_capacity, copy._capacity);
    ft_swap(this->_size, copy._size);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(ft_map<Key, MappedType>&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool    other_thread_safe = false;
    bool    other_lock_acquired = false;
    int     lock_error;

    this->clear();
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_capacity = 0;
    this->_size = 0;
    this->teardown_thread_safety();
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other_thread_safe = (other._mutex != ft_nullptr);
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
        {
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::~ft_map()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::ensure_capacity(size_t desired_capacity)
{
    if (desired_capacity <= this->_capacity)
        return (true);
    void *raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * desired_capacity);

    if (raw_memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    Pair<Key, MappedType> *new_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    size_t index = 0;

    while (index < this->_size)
    {
        construct_at(&new_data[index], ft_move(this->_data[index]));
        destroy_at(&this->_data[index]);
        index++;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = desired_capacity;
    return (true);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::find_index(const Key& key) const
{
    size_t index = 0;

    while (index < this->_size)
    {
        if (this->_data[index].key == key)
            return (index);
        index++;
    }
    return (this->_size);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::destroy_all_unlocked()
{
    size_t index = 0;

    while (index < this->_size)
    {
        destroy_at(&this->_data[index]);
        index++;
    }
    this->_size = 0;
    return ;
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::end_pointer_unlocked()
{
    if (this->_data == ft_nullptr)
        return (ft_nullptr);
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end_pointer_unlocked() const
{
    if (this->_data == ft_nullptr)
        return (ft_nullptr);
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock_internal(bool *lock_acquired) const
{
    int result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    size_t index = this->find_index(key);

    if (index != this->_size)
    {
        this->_data[index].value = value;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t next_capacity;

    if (this->_capacity == 0)
        next_capacity = 1;
    else
    {
        next_capacity = this->_capacity * 2;
        if (next_capacity <= this->_capacity)
        {
            size_t incremented_capacity = this->_capacity + 1;
            if (incremented_capacity == 0)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = incremented_capacity;
        }
    }
    if (!this->ensure_capacity(next_capacity))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, value));
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, MappedType&& value)
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    size_t index = this->find_index(key);

    if (index != this->_size)
    {
        this->_data[index].value = ft_move(value);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t next_capacity;

    if (this->_capacity == 0)
        next_capacity = 1;
    else
    {
        next_capacity = this->_capacity * 2;
        if (next_capacity <= this->_capacity)
        {
            size_t incremented_capacity = this->_capacity + 1;
            if (incremented_capacity == 0)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = incremented_capacity;
        }
    }
    if (!this->ensure_capacity(next_capacity))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, ft_move(value)));
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key)
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);
    Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (this->end_pointer_unlocked());
    }
    size_t index = this->find_index(key);

    if (index == this->_size)
        result = this->end_pointer_unlocked();
    else
        result = &this->_data[index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key) const
{
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    const Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (this->end_pointer_unlocked());
    }
    size_t index = this->find_index(key);

    if (index == this->_size)
        result = this->end_pointer_unlocked();
    else
        result = &this->_data[index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::remove(const Key& key)
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    destroy_at(&this->_data[index]);
    if (index != this->_size - 1)
    {
        construct_at(&this->_data[index], ft_move(this->_data[this->_size - 1]));
        destroy_at(&this->_data[this->_size - 1]);
    }
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::empty() const
{
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    bool    is_empty;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    is_empty = (this->_size == 0);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::clear()
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->destroy_all_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::size() const
{
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    size_t  current_size;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_size = this->_size;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::capacity() const
{
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    size_t  current_capacity;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_capacity = this->_capacity;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::end()
{
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);
    Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (this->end_pointer_unlocked());
    }
    result = this->end_pointer_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end() const
{
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    const Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (this->end_pointer_unlocked());
    }
    result = this->end_pointer_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
MappedType& ft_map<Key, MappedType>::at(const Key& key)
{
    static MappedType    error_value = MappedType();
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_value);
    }
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (this->_data[index].value);
}

template <typename Key, typename MappedType>
const MappedType& ft_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType    error_value = MappedType();
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_value);
    }
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (this->_data[index].value);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();

    ft_global_error_stack_push(result);
    return (result);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::is_thread_safe_enabled() const
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::unlock(bool lock_acquired) const
{
    int unlock_error = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename Key, typename MappedType>
#ifdef LIBFT_TEST_BUILD
template <typename Key, typename MappedType>
pt_recursive_mutex* ft_map<Key, MappedType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
