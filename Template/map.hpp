#ifndef FT_MAP_HPP
#define FT_MAP_HPP

#include "pair.hpp"
#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include "move.hpp"

template <typename Key, typename MappedType>
class ft_map
{
    private:
        Pair<Key, MappedType>*            _data;
        size_t                           _capacity;
        size_t                           _size;
        mutable ft_operation_error_stack _operation_errors;
        mutable pt_mutex*                _state_mutex;
        bool                             _thread_safe_enabled;

        void    record_operation_error(int error_code) const noexcept;
        int     resize_unlocked(size_t new_capacity);
        size_t  find_index_unlocked(const Key& key) const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     unlock_mutex(pt_mutex *mutex, bool lock_acquired) const;
        void    teardown_thread_safety();
        Pair<Key, MappedType>*  get_end_pointer_unlocked();
        const Pair<Key, MappedType>* get_end_pointer_unlocked() const;

    public:
        ft_map(size_t initial_capacity = 10);
        ft_map(const ft_map& other);
        ft_map& operator=(const ft_map& other);
        ft_map(ft_map&& other) noexcept;
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
        void*       get_mutex_address_debug() const;
        int         last_operation_error() const noexcept;
        const char* last_operation_error_str() const noexcept;
        int         operation_error_at(ft_size_t index) const noexcept;
        const char* operation_error_str_at(ft_size_t index) const noexcept;
        void        pop_operation_errors() noexcept;
        int         pop_oldest_operation_error() noexcept;
        int         pop_newest_operation_error() noexcept;
};

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::last_operation_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

template <typename Key, typename MappedType>
const char* ft_map<Key, MappedType>::last_operation_error_str() const noexcept
{
    int error_code;
    const char *error_string;

    error_code = this->last_operation_error();
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::operation_error_at(ft_size_t index) const noexcept
{
    return (ft_operation_error_stack_error_at(&this->_operation_errors, index));
}

template <typename Key, typename MappedType>
const char* ft_map<Key, MappedType>::operation_error_str_at(ft_size_t index) const noexcept
{
    int error_code;
    const char *error_string;

    error_code = this->operation_error_at(index);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::pop_operation_errors() noexcept
{
    ft_operation_error_stack_pop_all(&this->_operation_errors);
    ft_global_error_stack_pop_all();
    return ;
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::pop_oldest_operation_error() noexcept
{
    ft_global_error_stack_pop_last();
    return (ft_operation_error_stack_pop_last(&this->_operation_errors));
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::pop_newest_operation_error() noexcept
{
    ft_global_error_stack_pop_newest();
    return (ft_operation_error_stack_pop_newest(&this->_operation_errors));
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(size_t initial_capacity)
    : _data(ft_nullptr), _capacity(initial_capacity), _size(0),
      _operation_errors({{}, {}, 0}), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    void *raw_memory;

    if (this->_capacity == 0)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return ;
    }
    raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);
    if (raw_memory == ft_nullptr)
    {
        this->_capacity = 0;
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(const ft_map<Key, MappedType>& other)
    : _data(ft_nullptr), _capacity(0), _size(0),
      _operation_errors({{}, {}, 0}), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    bool other_thread_safe;
    size_t index;
    void *raw_memory;
    int lock_error;

    other_lock_acquired = false;
    other_thread_safe = false;
    raw_memory = ft_nullptr;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_capacity = other._capacity;
    this->_size = other._size;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    if (this->_capacity > 0)
    {
        raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);
        if (raw_memory == ft_nullptr)
        {
            other.unlock_internal(other_lock_acquired);
            this->_capacity = 0;
            this->_size = 0;
            this->record_operation_error(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
        index = 0;
        while (index < this->_size)
        {
            construct_at(&this->_data[index], other._data[index]);
            index++;
        }
    }
    other.unlock_internal(other_lock_acquired);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            size_t cleanup_index;

            cleanup_index = 0;
            if (this->_data != ft_nullptr)
            {
                while (cleanup_index < this->_size)
                {
                    ::destroy_at(&this->_data[cleanup_index]);
                    cleanup_index++;
                }
                cma_free(this->_data);
            }
            this->_data = ft_nullptr;
            this->_capacity = 0;
            this->_size = 0;
            return ;
        }
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(const ft_map<Key, MappedType>& other)
{
    ft_map<Key, MappedType> copy(other);
    bool this_lock_acquired;
    Pair<Key, MappedType> *previous_data;
    size_t previous_size;
    size_t previous_capacity;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;
    int lock_error;

    if (this == &other)
        return (*this);
    if (copy.last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(copy.last_operation_error());
        return (*this);
    }
    this_lock_acquired = false;
    lock_error = this->lock_internal(&this_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    previous_data = this->_data;
    previous_size = this->_size;
    previous_capacity = this->_capacity;
    previous_mutex = this->_state_mutex;
    previous_thread_safe = this->_thread_safe_enabled;
    this->_data = copy._data;
    this->_size = copy._size;
    this->_capacity = copy._capacity;
    this->_state_mutex = copy._state_mutex;
    this->_thread_safe_enabled = copy._thread_safe_enabled;
    copy._data = previous_data;
    copy._size = previous_size;
    copy._capacity = previous_capacity;
    copy._state_mutex = previous_mutex;
    copy._thread_safe_enabled = previous_thread_safe;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(this_lock_acquired);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(ft_map<Key, MappedType>&& other) noexcept
    : _data(ft_nullptr), _capacity(0), _size(0),
      _operation_errors({{}, {}, 0}), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    int lock_error;

    other_lock_acquired = false;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    this->_state_mutex = other._state_mutex;
    this->_thread_safe_enabled = other._thread_safe_enabled;
    other.unlock_internal(other_lock_acquired);
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(ft_map<Key, MappedType>&& other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    Pair<Key, MappedType> *previous_data;
    size_t previous_size;
    pt_mutex *previous_mutex;
    pt_mutex *this_locked_mutex;
    bool previous_thread_safe;
    bool other_thread_safe;
    bool new_mutex_locked;
    int lock_error;
    int other_lock_error;

    if (this == &other)
        return (*this);
    this_lock_acquired = false;
    lock_error = this->lock_internal(&this_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    other_lock_acquired = false;
    other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        this->record_operation_error(other_lock_error);
        return (*this);
    }
    new_mutex_locked = false;
    previous_data = this->_data;
    previous_size = this->_size;
    previous_mutex = this->_state_mutex;
    this_locked_mutex = this->_state_mutex;
    previous_thread_safe = this->_thread_safe_enabled;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    this->_state_mutex = other._state_mutex;
    this->_thread_safe_enabled = other._thread_safe_enabled;
    other.unlock_internal(other_lock_acquired);
    this->unlock_mutex(this_locked_mutex, this_lock_acquired);
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    if (previous_data != ft_nullptr && previous_data != this->_data)
    {
        size_t index;

        index = 0;
        while (index < previous_size)
        {
            ::destroy_at(&previous_data[index]);
            index++;
        }
        cma_free(previous_data);
    }
    if (previous_thread_safe && previous_mutex != ft_nullptr && previous_mutex != this->_state_mutex)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    if (other_thread_safe && this->_state_mutex == ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
            return (*this);
        if (this->_state_mutex != ft_nullptr)
        {
            this->_state_mutex->lock(THREAD_ID);
            int mutex_error = this->_state_mutex->operation_error_last_error();
            if (mutex_error != FT_ERR_SUCCESSS)
            {
                this->record_operation_error(mutex_error);
                return (*this);
            }
            new_mutex_locked = true;
        }
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_mutex(this->_state_mutex, new_mutex_locked);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::~ft_map()
{
    bool lock_acquired;
    Pair<Key, MappedType> *data_pointer;
    size_t stored_size;

    lock_acquired = false;
    data_pointer = this->_data;
    stored_size = this->_size;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESSS)
    {
        this->_data = ft_nullptr;
        this->_capacity = 0;
        this->_size = 0;
        int unlock_error;

        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESSS)
            this->record_operation_error(unlock_error);
        else
            this->record_operation_error(FT_ERR_SUCCESSS);
    }
    else
        this->record_operation_error(lock_error);
    if (data_pointer != ft_nullptr)
    {
        size_t index;

        index = 0;
        while (index < stored_size)
        {
            ::destroy_at(&data_pointer[index]);
            index++;
        }
        cma_free(data_pointer);
    }
    this->teardown_thread_safety();
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    bool lock_acquired;
    size_t index;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    index = this->find_index_unlocked(key);
    if (index != this->_size)
    {
        this->_data[index].value = value;
        this->record_operation_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        size_t next_capacity;

        if (this->_capacity == 0)
            next_capacity = 1;
        else
        {
            size_t doubled_capacity;

            doubled_capacity = this->_capacity * 2;
            if (doubled_capacity <= this->_capacity)
            {
                this->record_operation_error(FT_ERR_NO_MEMORY);
                this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = doubled_capacity;
        }
        int resize_error;

        resize_error = this->resize_unlocked(next_capacity);
        if (resize_error != FT_ERR_SUCCESSS)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, ft_move(value)));
    this->_size++;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, MappedType&& value)
{
    bool lock_acquired;
    size_t index;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    index = this->find_index_unlocked(key);
    if (index != this->_size)
    {
        this->_data[index].value = ft_move(value);
        this->record_operation_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        size_t next_capacity;

        if (this->_capacity == 0)
            next_capacity = 1;
        else
        {
            size_t doubled_capacity;

            doubled_capacity = this->_capacity * 2;
            if (doubled_capacity <= this->_capacity)
            {
                this->record_operation_error(FT_ERR_NO_MEMORY);
                this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = doubled_capacity;
        }
        int resize_error;

        resize_error = this->resize_unlocked(next_capacity);
        if (resize_error != FT_ERR_SUCCESSS)
        {
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], Pair<Key, MappedType>(key, ft_move(value)));
    this->_size++;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key)
{
    bool lock_acquired;
    size_t index;
    Pair<Key, MappedType> *result;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        result = this->get_end_pointer_unlocked();
        return (result);
    }
    index = this->find_index_unlocked(key);
    if (index == this->_size)
    {
        result = this->get_end_pointer_unlocked();
        this->record_operation_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (result);
    }
    result = &this->_data[index];
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key) const
{
    bool lock_acquired;
    size_t index;
    const Pair<Key, MappedType> *result;

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        result = this->get_end_pointer_unlocked();
        return (result);
    }
    index = this->find_index_unlocked(key);
    if (index == this->_size)
    {
        result = this->get_end_pointer_unlocked();
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
        const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
        return (result);
    }
    result = &this->_data[index];
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::remove(const Key& key)
{
    bool lock_acquired;
    size_t index;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    index = this->find_index_unlocked(key);
    if (index == this->_size)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    ::destroy_at(&this->_data[index]);
    if (index != this->_size - 1)
    {
        construct_at(&this->_data[index], ft_move(this->_data[this->_size - 1]));
        ::destroy_at(&this->_data[this->_size - 1]);
    }
    this->_size--;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::empty() const
{
    bool lock_acquired;
    bool is_empty;

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        return (true);
    }
    is_empty = (this->_size == 0);
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::clear()
{
    bool lock_acquired;
    size_t index;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    index = 0;
    while (index < this->_size)
    {
        ::destroy_at(&this->_data[index]);
        index++;
    }
    this->_size = 0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::size() const
{
    bool lock_acquired;
    size_t current_size;

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::capacity() const
{
    bool lock_acquired;
    size_t current_capacity;

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        return (0);
    }
    current_capacity = this->_capacity;
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::end()
{
    bool lock_acquired;
    Pair<Key, MappedType> *result;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        result = this->get_end_pointer_unlocked();
        return (result);
    }
    result = this->get_end_pointer_unlocked();
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end() const
{
    bool lock_acquired;
    const Pair<Key, MappedType> *result;

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        result = this->get_end_pointer_unlocked();
        return (result);
    }
    result = this->get_end_pointer_unlocked();
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::get_end_pointer_unlocked()
{
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::get_end_pointer_unlocked() const
{
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
MappedType& ft_map<Key, MappedType>::at(const Key& key)
{
    bool lock_acquired;
    size_t index;
    static MappedType error_value = MappedType();

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (error_value);
    }
    index = this->find_index_unlocked(key);
    if (this->_size == 0 || index == this->_size)
    {
        this->record_operation_error(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (this->_data[index].value);
}

template <typename Key, typename MappedType>
const MappedType& ft_map<Key, MappedType>::at(const Key& key) const
{
    bool lock_acquired;
    size_t index;
    static MappedType error_value = MappedType();

    lock_acquired = false;
    int lock_error;

    lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(lock_error);
        return (error_value);
    }
    index = this->find_index_unlocked(key);
    if (this->_size == 0 || index == this->_size)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_INTERNAL);
        const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
        return (error_value);
    }
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    const_cast<ft_map<Key, MappedType> *>(this)->unlock_internal(lock_acquired);
    return (this->_data[index].value);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::enable_thread_safety()
{
    void *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    int mutex_error;

    mutex_error = mutex_pointer->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->record_operation_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock(bool *lock_acquired) const
{
    int error_code;

    error_code = this->lock_internal(lock_acquired);
    if (error_code != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(error_code);
        return (-1);
    }
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(unlock_error);
        return ;
    }
    const_cast<ft_map<Key, MappedType> *>(this)->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
void* ft_map<Key, MappedType>::get_mutex_address_debug() const
{
    return (this->_state_mutex);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::resize_unlocked(size_t new_capacity)
{
    Pair<Key, MappedType> *new_data;
    void *raw_memory;
    size_t index;

    raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * new_capacity);
    if (raw_memory == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    new_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    index = 0;
    while (index < this->_size)
    {
        construct_at(&new_data[index], ft_move(this->_data[index]));
        ::destroy_at(&this->_data[index]);
        index++;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::find_index_unlocked(const Key& key) const
{
    size_t index;

    index = 0;
    while (index < this->_size)
    {
        if (this->_data[index].key == key)
            return (index);
        index++;
    }
    return (this->_size);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESSS);
    }
    this->_state_mutex->lock(THREAD_ID);
    int mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_state_mutex->unlock(THREAD_ID);
    return (this->_state_mutex->operation_error_last_error());
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::unlock_mutex(pt_mutex *mutex, bool lock_acquired) const
{
    if (!lock_acquired || mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex->unlock(THREAD_ID);
    return (mutex->operation_error_last_error());
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
