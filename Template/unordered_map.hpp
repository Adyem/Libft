#ifndef FT_UNORDERED_MAP_HPP
#define FT_UNORDERED_MAP_HPP

#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <functional>
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "move.hpp"

static inline int ft_unordered_map_last_global_error() noexcept
{
    return (ft_global_error_stack_peek_last_error());
}

template <typename Key, typename MappedType>
struct ft_pair
{
    Key         first;
    MappedType  second;
    ft_pair();
    ft_pair(const Key& k, const MappedType& m);
};

template <typename Key, typename MappedType>
class ft_unordered_map
{
    private:
        ft_pair<Key, MappedType>*  _data;
        bool*                      _occupied;
        size_t                     _capacity;
        size_t                     _size;
        mutable pt_recursive_mutex* _mutex;
        int     resize(size_t new_capacity);
        size_t  find_index(const Key& key) const;
        size_t  hash_key(const Key& key) const;
        int     insert_internal(const Key& key, const MappedType& value);
        bool    has_storage() const;
        void    flag_storage_error() const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        class iterator
        {
            private:
                ft_pair<Key, MappedType>* _data;
                bool*                     _occupied;
                size_t                    _index;
                size_t                    _capacity;
                mutable pt_recursive_mutex* _mutex;

                void                      advance_to_valid_index_unlocked();
                int                       lock_internal(bool *lock_acquired) const;
                int                       unlock_internal(bool lock_acquired) const;
                void                      teardown_thread_safety();

            public:
                iterator(ft_pair<Key, MappedType>* data, bool* occ, size_t idx, size_t cap);
                iterator(const iterator& other);
                iterator(iterator&& other) noexcept;
                iterator& operator=(const iterator& other);
                iterator& operator=(iterator&& other) noexcept;
                ~iterator();
                ft_pair<Key, MappedType>& operator*() const;
                ft_pair<Key, MappedType>* operator->() const;
                iterator& operator++();
                bool operator==(const iterator& other) const;
                bool operator!=(const iterator& other) const;
                int  enable_thread_safety();
                void disable_thread_safety();
                bool is_thread_safe() const;
                int  lock(bool *lock_acquired) const;
                void unlock(bool lock_acquired) const;
                int  get_error() const;
                const char* get_error_str() const;
#ifdef LIBFT_TEST_BUILD
                pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        };

        class const_iterator
        {
            private:
                const ft_pair<Key, MappedType>* _data;
                const bool*                     _occupied;
                size_t                          _index;
                size_t                          _capacity;
                mutable pt_recursive_mutex*     _mutex;

                void                            ft_global_error_stack_push(int error) const;
                void                            advance_to_valid_index_unlocked();
                int                             lock_internal(bool *lock_acquired) const;
                int                             unlock_internal(bool lock_acquired) const;
                void                            teardown_thread_safety();

            public:
                const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap);
                const_iterator(const const_iterator& other);
                const_iterator(const_iterator&& other) noexcept;
                const_iterator& operator=(const const_iterator& other);
                const_iterator& operator=(const_iterator&& other) noexcept;
                ~const_iterator();
                const ft_pair<Key, MappedType>& operator*() const;
                const ft_pair<Key, MappedType>* operator->() const;
                const_iterator& operator++();
                bool operator==(const const_iterator& other) const;
                bool operator!=(const const_iterator& other) const;
                int  enable_thread_safety();
                void disable_thread_safety();
                bool is_thread_safe() const;
                int  lock(bool *lock_acquired) const;
                void unlock(bool lock_acquired) const;
                int  get_error() const;
                const char* get_error_str() const;
#ifdef LIBFT_TEST_BUILD
                pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
        };

        ft_unordered_map(size_t initial_capacity = 10);
        ft_unordered_map(const ft_unordered_map& other);
        ft_unordered_map& operator=(const ft_unordered_map& other);
        ft_unordered_map(ft_unordered_map&& other) noexcept;
        ft_unordered_map& operator=(ft_unordered_map&& other) noexcept;
        ~ft_unordered_map();

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        void           insert(const Key& key, const MappedType& value);
        iterator       find(const Key& key);
        const_iterator find(const Key& key) const;
        void           erase(const Key& key);
        bool           empty() const;
        void           clear();
        size_t         size() const;
        size_t         bucket_count() const;
        bool           has_valid_storage() const;
        static int     last_operation_error() noexcept;
        static const char* last_operation_error_str() noexcept;
        static int     operation_error_at(ft_size_t index) noexcept;
        static const char* operation_error_str_at(ft_size_t index) noexcept;
        static void    pop_operation_errors() noexcept;
        static int     pop_oldest_operation_error() noexcept;
        static int     pop_newest_operation_error() noexcept;
        iterator       begin();
        iterator       end();
        const_iterator begin() const;
        const_iterator end() const;
        MappedType&    at(const Key& key);
        const MappedType& at(const Key& key) const;
        MappedType&    operator[](const Key& key);
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair() : first(), second()
{
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair(const Key& k, const MappedType& m) : first(k), second(m)
{
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::iterator(ft_pair<Key, MappedType>* data, bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap),
{
    this->advance_to_valid_index_unlocked();
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::iterator(const iterator& other)
    : _data(other._data), _occupied(other._occupied), _index(other._index),
      _capacity(other._capacity),
{
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::iterator(iterator&& other) noexcept
    : _data(other._data), _occupied(other._occupied), _index(other._index),
      _capacity(other._capacity),
{
    bool other_thread_safe;

    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.teardown_thread_safety();
            other._data = ft_nullptr;
            other._occupied = ft_nullptr;
            other._index = 0;
            other._capacity = 0;
            return ;
        }
    }
    other.teardown_thread_safety();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    return ;
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator&
ft_unordered_map<Key, MappedType>::iterator::operator=(const iterator& other)
{
    if (this != &other)
    {
        this->teardown_thread_safety();
        this->_data = other._data;
        this->_occupied = other._occupied;
        this->_index = other._index;
        this->_capacity = other._capacity;
        this->_mutex = ft_nullptr;
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
        this->advance_to_valid_index_unlocked();
    }
    return (*this);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator&
ft_unordered_map<Key, MappedType>::iterator::operator=(iterator&& other) noexcept
{
    if (this != &other)
    {
        bool other_thread_safe;

        this->teardown_thread_safety();
        this->_data = other._data;
        this->_occupied = other._occupied;
        this->_index = other._index;
        this->_capacity = other._capacity;
        this->_mutex = ft_nullptr;
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
            {
                other.teardown_thread_safety();
                other._data = ft_nullptr;
                other._occupied = ft_nullptr;
                other._index = 0;
                other._capacity = 0;
                return (*this);
            }
        }
        other._data = ft_nullptr;
        other._occupied = ft_nullptr;
        other._index = 0;
        other._capacity = 0;
        other.teardown_thread_safety();
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::~iterator()
{
    this->teardown_thread_safety();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::ft_global_error_stack_push(int error) const
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error, operation_id);
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::advance_to_valid_index_unlocked()
{
    if (this->_occupied == ft_nullptr)
        return ;
    while (this->_index < this->_capacity && !this->_occupied[this->_index])
        this->_index++;
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
        return (FT_ERR_SUCCESSS);
    this->_mutex->lock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_mutex->unlock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    return (mutex_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_recursive_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>& ft_unordered_map<Key, MappedType>::iterator::operator*() const
{
    static ft_pair<Key, MappedType> error_value = ft_pair<Key, MappedType>();
    ft_pair<Key, MappedType>*       pair_pointer;
    bool                            lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_value);
    }
    if (this->_data == ft_nullptr || this->_occupied == ft_nullptr || this->_index >= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    if (!this->_occupied[this->_index])
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    pair_pointer = &this->_data[this->_index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*pair_pointer);
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>* ft_unordered_map<Key, MappedType>::iterator::operator->() const
{
    static ft_pair<Key, MappedType> error_value = ft_pair<Key, MappedType>();
    ft_pair<Key, MappedType>*       pair_pointer;
    bool                            lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (&error_value);
    }
    if (this->_data == ft_nullptr || this->_occupied == ft_nullptr || this->_index >= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (&error_value);
    }
    if (!this->_occupied[this->_index])
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (&error_value);
    }
    pair_pointer = &this->_data[this->_index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (pair_pointer);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator& ft_unordered_map<Key, MappedType>::iterator::operator++()
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    if (this->_index < this->_capacity)
        this->_index++;
    this->advance_to_valid_index_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator==(const iterator& other) const
{
    bool lock_acquired;
    bool equal;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    equal = (this->_data == other._data && this->_index == other._index);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (equal);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator!=(const iterator& other) const
{
    bool result;

    result = !(*this == other);
    return (result);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::enable_thread_safety()
{
    void     *memory;
    pt_recursive_mutex *mutex_pointer;

    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_recursive_mutex));
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_recursive_mutex();
    {
        int mutex_error;

        mutex_error = ft_global_error_stack_drop_last_error();
        ft_global_error_stack_drop_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            mutex_pointer->~pt_recursive_mutex();
            cma_free(memory);
            ft_global_error_stack_push(mutex_error);
            return (-1);
        }
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::is_thread_safe() const
{
    bool enabled;

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        ft_global_error_stack_push(result);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::get_error() const
{
    return (ft_global_error_stack_peek_last_error());
}

template <typename Key, typename MappedType>
const char* ft_unordered_map<Key, MappedType>::iterator::get_error_str() const
{
    return (ft_global_error_stack_peek_last_error_str());
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator(const ft_pair<Key, MappedType>* data, const bool* occ, size_t idx, size_t cap)
    : _data(data), _occupied(occ), _index(idx), _capacity(cap),
{
    this->advance_to_valid_index_unlocked();
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator(const const_iterator& other)
    : _data(other._data), _occupied(other._occupied), _index(other._index),
      _capacity(other._capacity),
{
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator(const_iterator&& other) noexcept
    : _data(other._data), _occupied(other._occupied), _index(other._index),
      _capacity(other._capacity),
{
    bool other_thread_safe;

    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.teardown_thread_safety();
            other._data = ft_nullptr;
            other._occupied = ft_nullptr;
            other._index = 0;
            other._capacity = 0;
            return ;
        }
    }
    other.teardown_thread_safety();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    return ;
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator&
ft_unordered_map<Key, MappedType>::const_iterator::operator=(const const_iterator& other)
{
    if (this != &other)
    {
        this->teardown_thread_safety();
        this->_data = other._data;
        this->_occupied = other._occupied;
        this->_index = other._index;
        this->_capacity = other._capacity;
        this->_mutex = ft_nullptr;
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
        this->advance_to_valid_index_unlocked();
    }
    return (*this);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator&
ft_unordered_map<Key, MappedType>::const_iterator::operator=(const_iterator&& other) noexcept
{
    if (this != &other)
    {
        bool other_thread_safe;

        this->teardown_thread_safety();
        this->_data = other._data;
        this->_occupied = other._occupied;
        this->_index = other._index;
        this->_capacity = other._capacity;
        this->_mutex = ft_nullptr;
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
            {
                other.teardown_thread_safety();
                other._data = ft_nullptr;
                other._occupied = ft_nullptr;
                other._index = 0;
                other._capacity = 0;
                return (*this);
            }
        }
        other._data = ft_nullptr;
        other._occupied = ft_nullptr;
        other._index = 0;
        other._capacity = 0;
        other.teardown_thread_safety();
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::~const_iterator()
{
    this->teardown_thread_safety();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::ft_global_error_stack_push(int error) const
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error, operation_id);
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::advance_to_valid_index_unlocked()
{
    if (this->_occupied == ft_nullptr)
        return ;
    while (this->_index < this->_capacity && !this->_occupied[this->_index])
        this->_index++;
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
        return (FT_ERR_SUCCESSS);
    this->_mutex->lock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_mutex->unlock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    return (mutex_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_recursive_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    return ;
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>& ft_unordered_map<Key, MappedType>::const_iterator::operator*() const
{
    static ft_pair<Key, MappedType> error_value = ft_pair<Key, MappedType>();
    const ft_pair<Key, MappedType>* pair_pointer;
    bool                            lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_value);
    }
    if (this->_data == ft_nullptr || this->_occupied == ft_nullptr || this->_index >= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    if (!this->_occupied[this->_index])
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    pair_pointer = &this->_data[this->_index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*pair_pointer);
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>* ft_unordered_map<Key, MappedType>::const_iterator::operator->() const
{
    static ft_pair<Key, MappedType> error_value = ft_pair<Key, MappedType>();
    const ft_pair<Key, MappedType>* pair_pointer;
    bool                            lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (&error_value);
    }
    if (this->_data == ft_nullptr || this->_occupied == ft_nullptr || this->_index >= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (&error_value);
    }
    if (!this->_occupied[this->_index])
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (&error_value);
    }
    pair_pointer = &this->_data[this->_index];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (pair_pointer);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator& ft_unordered_map<Key, MappedType>::const_iterator::operator++()
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    if (this->_index < this->_capacity)
        this->_index++;
    this->advance_to_valid_index_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator==(const const_iterator& other) const
{
    bool lock_acquired;
    bool equal;

    lock_acquired = false;
    int lock_error;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    equal = (this->_data == other._data && this->_index == other._index);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (equal);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator!=(const const_iterator& other) const
{
    bool result;

    result = !(*this == other);
    return (result);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::enable_thread_safety()
{
    void     *memory;
    pt_recursive_mutex *mutex_pointer;

    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_recursive_mutex));
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_recursive_mutex();
    {
        int mutex_error;

        mutex_error = ft_global_error_stack_drop_last_error();
        ft_global_error_stack_drop_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            mutex_pointer->~pt_recursive_mutex();
            cma_free(memory);
            ft_global_error_stack_push(mutex_error);
            return (-1);
        }
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::is_thread_safe() const
{
    bool enabled;

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        ft_global_error_stack_push(result);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::get_error() const
{
    return (ft_global_error_stack_peek_last_error());
}

template <typename Key, typename MappedType>
const char* ft_unordered_map<Key, MappedType>::const_iterator::get_error_str() const
{
    return (ft_global_error_stack_peek_last_error_str());
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(size_t initial_capacity)
    : _data(ft_nullptr), _occupied(ft_nullptr), _capacity(initial_capacity),
{
    void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
    if (!raw_data)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    void* raw_occupied = cma_malloc(sizeof(bool) * _capacity);
    if (!raw_occupied)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        cma_free(_data);
        _data = ft_nullptr;
        _occupied = ft_nullptr;
        return ;
    }
    _occupied = static_cast<bool*>(raw_occupied);
    size_t i = 0;
    while (i < _capacity)
    {
        _occupied[i] = false;
        i++;
    }
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(const ft_unordered_map<Key, MappedType>& other)
    : _data(ft_nullptr), _occupied(ft_nullptr), _capacity(other._capacity),
{
    if (other._data != ft_nullptr && _size > 0)
    {
        void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * _capacity);
        if (!raw_data)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
        void* raw_occupied = cma_malloc(sizeof(bool) * _capacity);
        if (!raw_occupied)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            cma_free(_data);
            _data = ft_nullptr;
            _occupied = ft_nullptr;
            _size = 0;
            _capacity = 0;
            return ;
        }
        _occupied = static_cast<bool*>(raw_occupied);
        size_t i = 0;
        while (i < _capacity)
        {
            _occupied[i] = false;
            i++;
        }
        size_t index = 0;
        size_t count = 0;
        while (count < other._size && index < other._capacity)
        {
            if (other._occupied[index])
            {
                construct_at(&_data[index], other._data[index]);
                _occupied[index] = true;
                count++;
            }
            index++;
        }
    }
    else
    {
        _data = ft_nullptr;
        _occupied = ft_nullptr;
    }
    {
        this->enable_thread_safety();
    }
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(const ft_unordered_map<Key, MappedType>& other)
{
    if (this != &other)
    {
        if (this->_data != ft_nullptr)
        {
            size_t i = 0;
            size_t count = 0;
            while (count < this->_size && i < this->_capacity)
            {
                if (this->_occupied[i])
                {
                    ::destroy_at(&this->_data[i]);
                    count++;
                }
                i++;
            }
            cma_free(this->_data);
            cma_free(this->_occupied);
        }
        this->teardown_thread_safety();
        this->_data = ft_nullptr;
        this->_occupied = ft_nullptr;
        this->_capacity = other._capacity;
        this->_size = other._size;
        if (other._data != ft_nullptr && other._size > 0)
        {
            void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * other._capacity);
            if (!raw_data)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                this->_data = ft_nullptr;
                this->_occupied = ft_nullptr;
                this->_size = 0;
                this->_capacity = 0;
                return (*this);
            }
            this->_data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
            void* raw_occupied = cma_malloc(sizeof(bool) * other._capacity);
            if (!raw_occupied)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                cma_free(this->_data);
                this->_data = ft_nullptr;
                this->_occupied = ft_nullptr;
                this->_size = 0;
                this->_capacity = 0;
                return (*this);
            }
            this->_occupied = static_cast<bool*>(raw_occupied);
            size_t i = 0;
            while (i < this->_capacity)
            {
                this->_occupied[i] = false;
                i++;
            }
            size_t index = 0;
            size_t count = 0;
            while (count < other._size && index < other._capacity)
            {
                if (other._occupied[index])
                {
                    construct_at(&this->_data[index], other._data[index]);
                    this->_occupied[index] = true;
                    count++;
                }
                index++;
            }
        }
        else
        {
            this->_data = ft_nullptr;
            this->_occupied = ft_nullptr;
        }
        {
            this->enable_thread_safety();
        }
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(ft_unordered_map<Key, MappedType>&& other) noexcept
    : _data(other._data), _occupied(other._occupied), _capacity(other._capacity),
{
    bool other_thread_safe;

    if (other_thread_safe)
        this->enable_thread_safety();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.teardown_thread_safety();
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(ft_unordered_map<Key, MappedType>&& other) noexcept
{
    if (this != &other)
    {
        bool other_thread_safe;

        if (this->_data != ft_nullptr)
        {
            size_t i = 0;
            size_t count = 0;
            while (count < this->_size && i < this->_capacity)
            {
                if (this->_occupied[i])
                {
                    ::destroy_at(&this->_data[i]);
                    count++;
                }
                i++;
            }
            cma_free(this->_data);
            cma_free(this->_occupied);
        }
        this->teardown_thread_safety();
        this->_data = other._data;
        this->_occupied = other._occupied;
        this->_capacity = other._capacity;
        this->_size = other._size;
        if (other_thread_safe)
            this->enable_thread_safety();
        other._data = ft_nullptr;
        other._occupied = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other.teardown_thread_safety();
    }
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::~ft_unordered_map()
{
    if (this->_data != ft_nullptr && this->_occupied != ft_nullptr)
    {
        size_t i = 0;
        size_t count = 0;
        while (count < this->_size && i < this->_capacity)
        {
            if (this->_occupied[i])
            {
                ::destroy_at(&this->_data[i]);
                count++;
            }
            i++;
        }
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    if (this->_occupied != ft_nullptr)
        cma_free(this->_occupied);
    this->teardown_thread_safety();
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::enable_thread_safety()
{
    void     *memory;
    pt_recursive_mutex *mutex_pointer;

    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_recursive_mutex));
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_recursive_mutex();
    {
        int mutex_error;

        mutex_error = ft_global_error_stack_drop_last_error();
        ft_global_error_stack_drop_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            mutex_pointer->~pt_recursive_mutex();
            cma_free(memory);
            ft_global_error_stack_push(mutex_error);
            return (-1);
        }
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::is_thread_safe() const
{
    bool enabled;

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template<typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::ft_global_error_stack_push(int error) const
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error, operation_id);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_storage() const
{
    if (_capacity == 0)
        return (false);
    if (_data == ft_nullptr)
        return (false);
    if (_occupied == ft_nullptr)
        return (false);
    return (true);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::flag_storage_error() const
{
    ft_global_error_stack_push(FT_ERR_NO_MEMORY);
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
        return (FT_ERR_SUCCESSS);
    this->_mutex->lock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_mutex->unlock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    return (mutex_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_recursive_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::hash_key(const Key& key) const
{
    static std::hash<Key> hf;
    return (hf(key) % _capacity);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::find_index(const Key& key) const
{
    if (_capacity == 0)
        return (_capacity);
    if (_data == ft_nullptr)
        return (_capacity);
    if (_occupied == ft_nullptr)
        return (_capacity);
    if (_size == 0)
        return (_capacity);
    size_t start_index = hash_key(key);
    size_t i = start_index;
    while (true)
    {
        if (!_occupied[i])
            return (_capacity);
        if (_data[i].first == key)
            return (i);
        i = (i + 1) % _capacity;
        if (i == start_index)
            return (_capacity);
    }
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::resize(size_t new_capacity)
{
    if (new_capacity == 0)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    void* raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * new_capacity);
    if (!raw_data)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    ft_pair<Key, MappedType>* new_data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    void* raw_occupied = cma_malloc(sizeof(bool) * new_capacity);
    if (!raw_occupied)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        cma_free(new_data);
        return (FT_ERR_NO_MEMORY);
    }
    bool* new_occ = static_cast<bool*>(raw_occupied);
    size_t i = 0;
    while (i < new_capacity)
    {
        new_occ[i] = false;
        i++;
    }
    size_t old_capacity = _capacity;
    ft_pair<Key, MappedType>* old_data = _data;
    bool* old_occ = _occupied;
    _data = new_data;
    _occupied = new_occ;
    _capacity = new_capacity;
    size_t old_size = _size;
    _size = 0;
    i = 0;
    size_t count = 0;
    int operation_error = FT_ERR_SUCCESSS;
    while (count < old_size && i < old_capacity)
    {
        if (old_occ[i])
        {
            int insert_error = this->insert_internal(old_data[i].first, old_data[i].second);
            ::destroy_at(&old_data[i]);
            count++;
            if (operation_error == FT_ERR_SUCCESSS)
                operation_error = insert_error;
        }
        i++;
    }
    cma_free(old_data);
    cma_free(old_occ);
    if (operation_error == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    else
        ft_global_error_stack_push(operation_error);
    return (operation_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::last_operation_error() noexcept
{
    return (ft_global_error_stack_peek_last_error());
}

template <typename Key, typename MappedType>
const char* ft_unordered_map<Key, MappedType>::last_operation_error_str() noexcept
{
    return (ft_global_error_stack_peek_last_error_str());
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::operation_error_at(ft_size_t index) noexcept
{
    return (ft_global_error_stack_error_at(index));
}

template <typename Key, typename MappedType>
const char* ft_unordered_map<Key, MappedType>::operation_error_str_at(ft_size_t index) noexcept
{
    return (ft_global_error_stack_error_str_at(index));
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::pop_operation_errors() noexcept
{
    ft_global_error_stack_pop_all();
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::pop_oldest_operation_error() noexcept
{
    return (ft_global_error_stack_pop_last());
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::pop_newest_operation_error() noexcept
{
    return (ft_global_error_stack_drop_last_error());
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::insert_internal(const Key& key, const MappedType& value)
{
    if (_capacity == 0 || _data == ft_nullptr || _occupied == ft_nullptr)
    {
        flag_storage_error();
        return (FT_ERR_NO_MEMORY);
    }
    size_t idx = find_index(key);
    if (idx != _capacity)
    {
        _data[idx].second = value;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    if ((_size * 2) >= _capacity)
    {
        int resize_error = this->resize(this->_capacity * 2);
        if (resize_error != FT_ERR_SUCCESSS)
            return (resize_error);
    }
    size_t start_index = hash_key(key);
    size_t i = start_index;
    while (true)
    {
        if (!_occupied[i])
        {
            construct_at(&_data[i], ft_pair<Key, MappedType>(key, value));
            _occupied[i] = true;
            ++_size;
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (FT_ERR_SUCCESSS);
        }
        i = (i + 1) % _capacity;
        if (i == start_index)
            break;
    }
    ft_global_error_stack_push(FT_ERR_INTERNAL);
    return (FT_ERR_INTERNAL);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return ;
    }
    int insert_error = this->insert_internal(key, value);
    if (insert_error == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    else
        ft_global_error_stack_push(insert_error);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::find(const Key& key)
{
    bool     lock_acquired;
    size_t   idx;
    iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return (res);
    }
    idx = this->find_index(key);
    if (idx == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (res);
    }
    res = iterator(this->_data, this->_occupied, idx, this->_capacity);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::find(const Key& key) const
{
    bool           lock_acquired;
    size_t         idx;
    const_iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return (res);
    }
    idx = this->find_index(key);
    if (idx == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (res);
    }
    res = const_iterator(this->_data, this->_occupied, idx, this->_capacity);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::erase(const Key& key)
{
    bool   lock_acquired;
    size_t idx;
    size_t next;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return ;
    }
    idx = this->find_index(key);
    if (idx == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    ::destroy_at(&this->_data[idx]);
    this->_occupied[idx] = false;
    this->_size -= 1;
    next = (idx + 1) % this->_capacity;
    while (this->_occupied[next])
    {
        size_t h;

        h = this->hash_key(this->_data[next].first);
        if ((next > idx && (h <= idx || h > next)) || (next < idx && (h <= idx && h > next)))
        {
            construct_at(&this->_data[idx], ft_move(this->_data[next]));
            ::destroy_at(&this->_data[next]);
            this->_occupied[idx] = true;
            this->_occupied[next] = false;
            idx = next;
        }
        next = (next + 1) % this->_capacity;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::empty() const
{
    bool is_empty;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
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
void ft_unordered_map<Key, MappedType>::clear()
{
    bool   lock_acquired;
    size_t i;
    size_t count;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return ;
    }
    i = 0;
    count = 0;
    while (count < this->_size && i < this->_capacity)
    {
        if (this->_occupied[i])
        {
            ::destroy_at(&this->_data[i]);
            this->_occupied[i] = false;
            count++;
        }
        i++;
    }
    this->_size = 0;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::size() const
{
    size_t current_size;
    bool   lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
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
size_t ft_unordered_map<Key, MappedType>::bucket_count() const
{
    size_t bucket_total;
    bool   lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    bucket_total = this->_capacity;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (bucket_total);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_valid_storage() const
{
    bool valid;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    valid = this->has_storage();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (valid);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::begin()
{
    bool     lock_acquired;
    iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        {
            if (res.enable_thread_safety() != 0)
                ft_global_error_stack_push(res.get_error());
        }
        this->unlock_internal(lock_acquired);
        return (res);
    }
    res = iterator(this->_data, this->_occupied, 0, this->_capacity);
    {
        if (res.enable_thread_safety() != 0)
        {
            ft_global_error_stack_push(res.get_error());
            this->unlock_internal(lock_acquired);
            return (res);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::end()
{
    bool     lock_acquired;
    iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    {
        if (res.enable_thread_safety() != 0)
        {
            ft_global_error_stack_push(res.get_error());
            this->unlock_internal(lock_acquired);
            return (res);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::begin() const
{
    bool           lock_acquired;
    const_iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        {
            if (res.enable_thread_safety() != 0)
                ft_global_error_stack_push(res.get_error());
        }
        this->unlock_internal(lock_acquired);
        return (res);
    }
    res = const_iterator(this->_data, this->_occupied, 0, this->_capacity);
    {
        if (res.enable_thread_safety() != 0)
        {
            ft_global_error_stack_push(res.get_error());
            this->unlock_internal(lock_acquired);
            return (res);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator ft_unordered_map<Key, MappedType>::end() const
{
    bool           lock_acquired;
    const_iterator res(this->_data, this->_occupied, this->_capacity, this->_capacity);

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (res);
    }
    {
        if (res.enable_thread_safety() != 0)
        {
            ft_global_error_stack_push(res.get_error());
            this->unlock_internal(lock_acquired);
            return (res);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (res);
}

template <typename Key, typename MappedType>
MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key)
{
    static MappedType error_mapped_value = MappedType();
    MappedType      *value;
    bool             lock_acquired;
    size_t           idx;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_mapped_value);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return (error_mapped_value);
    }
    idx = this->find_index(key);
    if (idx == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_mapped_value);
    }
    value = &this->_data[idx].second;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename Key, typename MappedType>
const MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType error_mapped_value = MappedType();
    const MappedType *value;
    bool               lock_acquired;
    size_t             idx;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_mapped_value);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return (error_mapped_value);
    }
    idx = this->find_index(key);
    if (idx == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        this->unlock_internal(lock_acquired);
        return (error_mapped_value);
    }
    value = &this->_data[idx].second;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename Key, typename MappedType>
MappedType& ft_unordered_map<Key, MappedType>::operator[](const Key& key)
{
    static MappedType error_value = MappedType();
    bool             lock_acquired;
    size_t           idx;
    size_t           start_index;
    size_t           i;
    MappedType      *value;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_value);
    }
    if (!this->has_storage())
    {
        this->flag_storage_error();
        this->unlock_internal(lock_acquired);
        return (error_value);
    }
    idx = this->find_index(key);
    if (idx != this->_capacity)
    {
        value = &this->_data[idx].second;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (*value);
    }
    if ((this->_size * 2) >= this->_capacity)
    {
        int resize_error = this->resize(this->_capacity * 2);
        if (resize_error != FT_ERR_SUCCESSS)
        {
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(resize_error);
            return (error_value);
        }
    }
    start_index = this->hash_key(key);
    i = start_index;
    while (true)
    {
        if (!this->_occupied[i])
        {
            construct_at(&this->_data[i], ft_pair<Key, MappedType>(key, MappedType()));
            this->_occupied[i] = true;
            this->_size += 1;
            value = &this->_data[i].second;
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            this->unlock_internal(lock_acquired);
            return (*value);
        }
        i = (i + 1) % this->_capacity;
        if (i == start_index)
            break;
    }
    ft_global_error_stack_push(FT_ERR_INTERNAL);
    this->unlock_internal(lock_acquired);
    return (error_value);
}

template <typename Key, typename MappedType>
using ft_unord_map = ft_unordered_map<Key, MappedType>;

#endif
