#ifndef FT_UNORDERED_MAP_HPP
#define FT_UNORDERED_MAP_HPP

#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <new>

template <typename Key, typename MappedType>
struct ft_pair
{
    Key         first;
    MappedType  second;
    ft_pair();
    ft_pair(const Key& key, const MappedType& value);
};

template <typename Key, typename MappedType>
class ft_unordered_map
{
    private:
        ft_pair<Key, MappedType>      *_data;
        bool                          *_occupied;
        size_t                        _capacity;
        size_t                        _size;
        size_t                        _requested_capacity;
        mutable pt_recursive_mutex    *_mutex;
        mutable uint8_t               _initialized_state;

        static const uint8_t          _state_uninitialized = 0;
        static const uint8_t          _state_destroyed = 1;
        static const uint8_t          _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;
        static int  &operation_error_storage() noexcept;
        static void record_operation_error(int error_code) noexcept;

        bool    has_storage_unlocked() const;
        void    destroy_elements_unlocked();
        void    release_storage_unlocked();
        int     prepare_empty_storage_unlocked(size_t capacity);

        size_t  hash_key(const Key& key) const;
        size_t  find_index_unlocked(const Key& key) const;
        int     insert_internal_unlocked(const Key& key, const MappedType& value);
        int     resize_unlocked(size_t new_capacity);

        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;

    public:
        class mapped_proxy
        {
            private:
                ft_unordered_map<Key, MappedType> * _parent_map;
                MappedType *                        _mapped_value_pointer;
                int32_t                             _last_error;
                int32_t                             _is_valid;

            public:
                mapped_proxy();
                mapped_proxy(ft_unordered_map<Key, MappedType> *parent_map,
                    MappedType *mapped_value_pointer, int32_t error_code,
                    int32_t is_valid);
                ~mapped_proxy();

                MappedType *operator->();
                MappedType &operator*();
                mapped_proxy &operator=(const MappedType& mapped_value);
                operator MappedType() const;
                int32_t get_error() const;
                int32_t is_valid() const;
        };

        class iterator
        {
            private:
                ft_pair<Key, MappedType>      *_data;
                bool                          *_occupied;
                size_t                        _index;
                size_t                        _capacity;
                uint8_t                       _initialized_state;

                static thread_local int32_t   _last_error;
                static const uint8_t          _state_uninitialized = 0;
                static const uint8_t          _state_destroyed = 1;
                static const uint8_t          _state_initialized = 2;

                void    abort_lifecycle_error(const char *method_name,
                            const char *reason) const;
                void    abort_if_not_initialized(const char *method_name) const;

                void    advance_to_valid_index_unlocked();

            public:
                iterator();
                iterator(ft_pair<Key, MappedType>* data, bool* occupied,
                    size_t index, size_t capacity);
                iterator(const iterator& other) = delete;
                iterator(iterator&& other) = delete;
                iterator& operator=(const iterator& other) = delete;
                iterator& operator=(iterator&& other) = delete;
                ~iterator();

                int initialize(ft_pair<Key, MappedType>* data, bool* occupied,
                    size_t index, size_t capacity);
                int initialize(const iterator& other);
                int initialize(iterator&& other);
                int destroy();
                int move(iterator& other);
                int get_error() const;

                ft_pair<Key, MappedType>& operator*() const;
                ft_pair<Key, MappedType>* operator->() const;
                iterator& operator++();
                bool operator==(const iterator& other) const;
                bool operator!=(const iterator& other) const;
        };

        class const_iterator
        {
            private:
                const ft_pair<Key, MappedType>    *_data;
                const bool                        *_occupied;
                size_t                            _index;
                size_t                            _capacity;
                uint8_t                           _initialized_state;

                static thread_local int32_t       _last_error;
                static const uint8_t              _state_uninitialized = 0;
                static const uint8_t              _state_destroyed = 1;
                static const uint8_t              _state_initialized = 2;

                void    abort_lifecycle_error(const char *method_name,
                            const char *reason) const;
                void    abort_if_not_initialized(const char *method_name) const;

                void    advance_to_valid_index_unlocked();

            public:
                const_iterator();
                const_iterator(const ft_pair<Key, MappedType>* data,
                    const bool* occupied, size_t index, size_t capacity);
                const_iterator(const const_iterator& other) = delete;
                const_iterator(const_iterator&& other) = delete;
                const_iterator& operator=(const const_iterator& other) = delete;
                const_iterator& operator=(const_iterator&& other) = delete;
                ~const_iterator();

                int initialize(const ft_pair<Key, MappedType>* data,
                    const bool* occupied, size_t index, size_t capacity);
                int initialize(const const_iterator& other);
                int initialize(const_iterator&& other);
                int destroy();
                int move(const_iterator& other);
                int get_error() const;

                const ft_pair<Key, MappedType>& operator*() const;
                const ft_pair<Key, MappedType>* operator->() const;
                const_iterator& operator++();
                bool operator==(const const_iterator& other) const;
                bool operator!=(const const_iterator& other) const;
        };

        ft_unordered_map(size_t initial_capacity = 10);
        ft_unordered_map(const ft_unordered_map& other) = delete;
        ft_unordered_map& operator=(const ft_unordered_map& other);
        ft_unordered_map(ft_unordered_map&& other) noexcept = delete;
        ft_unordered_map& operator=(ft_unordered_map&& other) noexcept;
        ~ft_unordered_map();

        int     initialize();
        int     initialize(const ft_unordered_map& other);
        int     initialize(ft_unordered_map&& other);
        int     destroy();
        int     move(ft_unordered_map& other);

        int     enable_thread_safety();
        int     disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        void            insert(const Key& key, const MappedType& value);
        iterator        find(const Key& key);
        const_iterator  find(const Key& key) const;
        void            erase(const Key& key);
        bool            empty() const;
        void            clear();
        size_t          size() const;
        size_t          bucket_count() const;
        bool            has_valid_storage() const;

        static int          last_operation_error() noexcept;
        static const char   *last_operation_error_str() noexcept;
        static int          operation_error_at(ft_size_t index) noexcept;
        static const char   *operation_error_str_at(ft_size_t index) noexcept;
        static void         pop_operation_errors() noexcept;
        static int          pop_oldest_operation_error() noexcept;
        static int          pop_newest_operation_error() noexcept;

        iterator        begin();
        iterator        end();
        const_iterator  begin() const;
        const_iterator  end() const;

        MappedType          &at(const Key& key);
        const MappedType    &at(const Key& key) const;
        mapped_proxy        operator[](const Key& key);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair()
    : first(), second()
{
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>::ft_pair(const Key& key, const MappedType& value)
    : first(key), second(value)
{
    return ;
}

template <typename Key, typename MappedType>
int &ft_unordered_map<Key, MappedType>::operation_error_storage() noexcept
{
    static int error_code = FT_ERR_SUCCESS;

    return (error_code);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::record_operation_error(int error_code) noexcept
{
    ft_unordered_map<Key, MappedType>::operation_error_storage() = error_code;
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::abort_lifecycle_error(
    const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_unordered_map lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::abort_if_not_initialized(
    const char *method_name) const
{
    if (this->_initialized_state == ft_unordered_map<Key, MappedType>::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_storage_unlocked() const
{
    if (this->_data == ft_nullptr)
        return (false);
    if (this->_occupied == ft_nullptr)
        return (false);
    if (this->_capacity == 0)
        return (false);
    return (true);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::destroy_elements_unlocked()
{
    size_t index;
    size_t remaining_elements;

    if (!this->has_storage_unlocked())
        return ;
    index = 0;
    remaining_elements = this->_size;
    while (index < this->_capacity && remaining_elements > 0)
    {
        if (this->_occupied[index])
        {
            ::destroy_at(&this->_data[index]);
            this->_occupied[index] = false;
            remaining_elements -= 1;
        }
        index += 1;
    }
    this->_size = 0;
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::release_storage_unlocked()
{
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    if (this->_occupied != ft_nullptr)
        cma_free(this->_occupied);
    this->_data = ft_nullptr;
    this->_occupied = ft_nullptr;
    this->_capacity = 0;
    this->_size = 0;
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::prepare_empty_storage_unlocked(size_t capacity)
{
    void    *raw_data;
    void    *raw_occupied;
    size_t  index;

    if (capacity == 0)
        capacity = 1;
    raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * capacity);
    if (raw_data == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    raw_occupied = cma_malloc(sizeof(bool) * capacity);
    if (raw_occupied == ft_nullptr)
    {
        cma_free(raw_data);
        return (FT_ERR_NO_MEMORY);
    }
    this->_data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    this->_occupied = static_cast<bool*>(raw_occupied);
    this->_capacity = capacity;
    this->_size = 0;
    index = 0;
    while (index < this->_capacity)
    {
        this->_occupied[index] = false;
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::hash_key(const Key& key) const
{
    std::hash<Key> hasher;

    if (this->_capacity == 0)
        return (0);
    return (hasher(key) % this->_capacity);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::find_index_unlocked(const Key& key) const
{
    size_t  start_index;
    size_t  current_index;

    if (!this->has_storage_unlocked())
        return (this->_capacity);
    if (this->_size == 0)
        return (this->_capacity);
    start_index = this->hash_key(key);
    current_index = start_index;
    while (true)
    {
        if (!this->_occupied[current_index])
            return (this->_capacity);
        if (this->_data[current_index].first == key)
            return (current_index);
        current_index = (current_index + 1) % this->_capacity;
        if (current_index == start_index)
            return (this->_capacity);
    }
    return (this->_capacity);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::resize_unlocked(size_t new_capacity)
{
    ft_pair<Key, MappedType>  *old_data;
    bool                      *old_occupied;
    size_t                    old_capacity;
    size_t                    old_size;
    void                      *raw_data;
    void                      *raw_occupied;
    ft_pair<Key, MappedType>  *new_data;
    bool                      *new_occupied;
    size_t                    old_index;
    size_t                    inserted_count;

    if (new_capacity == 0)
        return (FT_ERR_NO_MEMORY);
    raw_data = cma_malloc(sizeof(ft_pair<Key, MappedType>) * new_capacity);
    if (raw_data == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    raw_occupied = cma_malloc(sizeof(bool) * new_capacity);
    if (raw_occupied == ft_nullptr)
    {
        cma_free(raw_data);
        return (FT_ERR_NO_MEMORY);
    }
    new_data = static_cast<ft_pair<Key, MappedType>*>(raw_data);
    new_occupied = static_cast<bool*>(raw_occupied);
    old_index = 0;
    while (old_index < new_capacity)
    {
        new_occupied[old_index] = false;
        old_index += 1;
    }

    old_data = this->_data;
    old_occupied = this->_occupied;
    old_capacity = this->_capacity;
    old_size = this->_size;

    inserted_count = 0;
    old_index = 0;
    while (old_index < old_capacity && inserted_count < old_size)
    {
        if (old_occupied[old_index])
        {
            size_t destination_index;
            std::hash<Key> hasher;

            destination_index = hasher(old_data[old_index].first) % new_capacity;
            while (new_occupied[destination_index])
                destination_index = (destination_index + 1) % new_capacity;
            construct_at(&new_data[destination_index], ft_move(old_data[old_index]));
            new_occupied[destination_index] = true;
            ::destroy_at(&old_data[old_index]);
            inserted_count += 1;
        }
        old_index += 1;
    }

    cma_free(old_data);
    cma_free(old_occupied);

    this->_data = new_data;
    this->_occupied = new_occupied;
    this->_capacity = new_capacity;
    this->_size = inserted_count;
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::insert_internal_unlocked(
    const Key& key, const MappedType& value)
{
    size_t key_index;
    size_t start_index;
    size_t current_index;

    if (!this->has_storage_unlocked())
        return (FT_ERR_INVALID_STATE);

    key_index = this->find_index_unlocked(key);
    if (key_index != this->_capacity)
    {
        this->_data[key_index].second = value;
        return (FT_ERR_SUCCESS);
    }

    if ((this->_size * 2) >= this->_capacity)
    {
        size_t new_capacity;
        int resize_error;

        new_capacity = this->_capacity * 2;
        if (new_capacity == 0)
            new_capacity = 1;
        resize_error = this->resize_unlocked(new_capacity);
        if (resize_error != FT_ERR_SUCCESS)
            return (resize_error);
    }

    start_index = this->hash_key(key);
    current_index = start_index;
    while (true)
    {
        if (!this->_occupied[current_index])
        {
            construct_at(&this->_data[current_index], ft_pair<Key, MappedType>(key, value));
            this->_occupied[current_index] = true;
            this->_size += 1;
            return (FT_ERR_SUCCESS);
        }
        current_index = (current_index + 1) % this->_capacity;
        if (current_index == start_index)
            return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_INTERNAL);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::ft_unordered_map(size_t initial_capacity)
    : _data(ft_nullptr)
    , _occupied(ft_nullptr)
    , _capacity(0)
    , _size(0)
    , _requested_capacity(initial_capacity)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_unordered_map<Key, MappedType>::_state_uninitialized)
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        ft_unordered_map<Key, MappedType>::record_operation_error(initialize_error);
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(
    const ft_unordered_map& other)
{
    this->abort_if_not_initialized("ft_unordered_map::operator=(const ft_unordered_map&)");
    other.abort_if_not_initialized("ft_unordered_map::operator=(const ft_unordered_map&) source");

    if (this == &other)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
        return (*this);
    }

    bool self_lock_acquired;
    bool other_lock_acquired;

    self_lock_acquired = false;
    other_lock_acquired = false;

    if (this->_mutex != ft_nullptr && other._mutex != ft_nullptr && this->_mutex != other._mutex)
    {
        pt_recursive_mutex *first_mutex;
        pt_recursive_mutex *second_mutex;

        first_mutex = this->_mutex;
        second_mutex = other._mutex;
        if (first_mutex > second_mutex)
        {
            pt_recursive_mutex *temporary_mutex;

            temporary_mutex = first_mutex;
            first_mutex = second_mutex;
            second_mutex = temporary_mutex;
        }
        int first_lock_error = first_mutex->lock();
        if (first_lock_error != FT_ERR_SUCCESS)
        {
            ft_unordered_map<Key, MappedType>::record_operation_error(first_lock_error);
            return (*this);
        }
        if (first_mutex == this->_mutex)
            self_lock_acquired = true;
        else
            other_lock_acquired = true;

        int second_lock_error = second_mutex->lock();
        if (second_lock_error != FT_ERR_SUCCESS)
        {
            if (self_lock_acquired)
                this->_mutex->unlock();
            if (other_lock_acquired)
                other._mutex->unlock();
            ft_unordered_map<Key, MappedType>::record_operation_error(second_lock_error);
            return (*this);
        }
        if (second_mutex == this->_mutex)
            self_lock_acquired = true;
        else
            other_lock_acquired = true;
    }
    else
    {
        if (this->_mutex != ft_nullptr)
        {
            int self_lock_error = this->_mutex->lock();
            if (self_lock_error != FT_ERR_SUCCESS)
            {
                ft_unordered_map<Key, MappedType>::record_operation_error(self_lock_error);
                return (*this);
            }
            self_lock_acquired = true;
        }
        if (other._mutex != ft_nullptr && other._mutex != this->_mutex)
        {
            int other_lock_error = other._mutex->lock();
            if (other_lock_error != FT_ERR_SUCCESS)
            {
                if (self_lock_acquired)
                    this->_mutex->unlock();
                ft_unordered_map<Key, MappedType>::record_operation_error(other_lock_error);
                return (*this);
            }
            other_lock_acquired = true;
        }
    }

    this->destroy_elements_unlocked();
    this->release_storage_unlocked();

    this->_requested_capacity = other._requested_capacity;
    if (other._capacity > 0)
    {
        int prepare_error = this->prepare_empty_storage_unlocked(other._capacity);
        if (prepare_error != FT_ERR_SUCCESS)
        {
            if (self_lock_acquired)
                this->_mutex->unlock();
            if (other_lock_acquired)
                other._mutex->unlock();
            ft_unordered_map<Key, MappedType>::record_operation_error(prepare_error);
            return (*this);
        }
    }

    size_t index;
    size_t remaining_elements;

    index = 0;
    remaining_elements = other._size;
    while (index < other._capacity && remaining_elements > 0)
    {
        if (other._occupied[index])
        {
            int insert_error;

            insert_error = this->insert_internal_unlocked(other._data[index].first,
                other._data[index].second);
            if (insert_error != FT_ERR_SUCCESS)
            {
                if (self_lock_acquired)
                    this->_mutex->unlock();
                if (other_lock_acquired)
                    other._mutex->unlock();
                ft_unordered_map<Key, MappedType>::record_operation_error(insert_error);
                return (*this);
            }
            remaining_elements -= 1;
        }
        index += 1;
    }

    if (self_lock_acquired)
        this->_mutex->unlock();
    if (other_lock_acquired)
        other._mutex->unlock();

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>& ft_unordered_map<Key, MappedType>::operator=(
    ft_unordered_map&& other) noexcept
{
    this->abort_if_not_initialized("ft_unordered_map::operator=(ft_unordered_map&&)");
    other.abort_if_not_initialized("ft_unordered_map::operator=(ft_unordered_map&&) source");

    if (this == &other)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
        return (*this);
    }

    bool self_lock_acquired;
    bool other_lock_acquired;

    self_lock_acquired = false;
    other_lock_acquired = false;

    if (this->_mutex != ft_nullptr && other._mutex != ft_nullptr && this->_mutex != other._mutex)
    {
        pt_recursive_mutex *first_mutex;
        pt_recursive_mutex *second_mutex;

        first_mutex = this->_mutex;
        second_mutex = other._mutex;
        if (first_mutex > second_mutex)
        {
            pt_recursive_mutex *temporary_mutex;

            temporary_mutex = first_mutex;
            first_mutex = second_mutex;
            second_mutex = temporary_mutex;
        }

        int first_lock_error = first_mutex->lock();
        if (first_lock_error != FT_ERR_SUCCESS)
        {
            ft_unordered_map<Key, MappedType>::record_operation_error(first_lock_error);
            return (*this);
        }
        if (first_mutex == this->_mutex)
            self_lock_acquired = true;
        else
            other_lock_acquired = true;

        int second_lock_error = second_mutex->lock();
        if (second_lock_error != FT_ERR_SUCCESS)
        {
            if (self_lock_acquired)
                this->_mutex->unlock();
            if (other_lock_acquired)
                other._mutex->unlock();
            ft_unordered_map<Key, MappedType>::record_operation_error(second_lock_error);
            return (*this);
        }
        if (second_mutex == this->_mutex)
            self_lock_acquired = true;
        else
            other_lock_acquired = true;
    }
    else
    {
        if (this->_mutex != ft_nullptr)
        {
            int self_lock_error = this->_mutex->lock();
            if (self_lock_error != FT_ERR_SUCCESS)
            {
                ft_unordered_map<Key, MappedType>::record_operation_error(self_lock_error);
                return (*this);
            }
            self_lock_acquired = true;
        }
        if (other._mutex != ft_nullptr && other._mutex != this->_mutex)
        {
            int other_lock_error = other._mutex->lock();
            if (other_lock_error != FT_ERR_SUCCESS)
            {
                if (self_lock_acquired)
                    this->_mutex->unlock();
                ft_unordered_map<Key, MappedType>::record_operation_error(other_lock_error);
                return (*this);
            }
            other_lock_acquired = true;
        }
    }

    this->destroy_elements_unlocked();
    this->release_storage_unlocked();

    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_capacity = other._capacity;
    this->_size = other._size;
    this->_requested_capacity = other._requested_capacity;

    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._capacity = 0;
    other._size = 0;

    if (self_lock_acquired)
        this->_mutex->unlock();
    if (other_lock_acquired)
        other._mutex->unlock();

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::~ft_unordered_map()
{
    if (this->_initialized_state == ft_unordered_map<Key, MappedType>::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_unordered_map::~ft_unordered_map",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_unordered_map<Key, MappedType>::_state_initialized)
        (void)this->destroy();
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::initialize()
{
    size_t initial_capacity;
    int    prepare_error;
    int    mutex_error;

    if (this->_initialized_state == ft_unordered_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_unordered_map::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }

    this->destroy_elements_unlocked();
    this->release_storage_unlocked();

    initial_capacity = this->_requested_capacity;
    if (initial_capacity == 0)
        initial_capacity = 1;

    prepare_error = this->prepare_empty_storage_unlocked(initial_capacity);
    if (prepare_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_unordered_map<Key, MappedType>::_state_destroyed;
        ft_unordered_map<Key, MappedType>::record_operation_error(prepare_error);
        return (prepare_error);
    }

    this->_initialized_state = ft_unordered_map<Key, MappedType>::_state_initialized;
    mutex_error = this->enable_thread_safety();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->destroy_elements_unlocked();
        this->release_storage_unlocked();
        this->_initialized_state = ft_unordered_map<Key, MappedType>::_state_destroyed;
        ft_unordered_map<Key, MappedType>::record_operation_error(mutex_error);
        return (mutex_error);
    }

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::initialize(const ft_unordered_map& other)
{
    int initialize_error;
    int copy_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->operator=(other);
    copy_error = ft_unordered_map<Key, MappedType>::last_operation_error();
    if (copy_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (copy_error);
    }
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::initialize(ft_unordered_map&& other)
{
    int initialize_error;
    int move_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (move_error);
    }
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::destroy()
{
    bool lock_acquired;
    int  lock_error;
    int  unlock_error;
    int  mutex_destroy_error;

    if (this->_initialized_state != ft_unordered_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_unordered_map::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (lock_error);
    }

    this->destroy_elements_unlocked();
    this->release_storage_unlocked();

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (unlock_error);
    }

    mutex_destroy_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
    {
        mutex_destroy_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }

    this->_initialized_state = ft_unordered_map<Key, MappedType>::_state_destroyed;
    ft_unordered_map<Key, MappedType>::record_operation_error(mutex_destroy_error);
    return (mutex_destroy_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::move(ft_unordered_map& other)
{
    this->abort_if_not_initialized("ft_unordered_map::move");
    other.abort_if_not_initialized("ft_unordered_map::move source");
    this->operator=(ft_move(other));
    return (ft_unordered_map<Key, MappedType>::last_operation_error());
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::enable_thread_safety()
{
    this->abort_if_not_initialized("ft_unordered_map::enable_thread_safety");

    if (this->_mutex != ft_nullptr)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }

    pt_recursive_mutex *mutex_pointer;
    int                initialize_error;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }

    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        ft_unordered_map<Key, MappedType>::record_operation_error(initialize_error);
        return (initialize_error);
    }

    this->_mutex = mutex_pointer;
    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::disable_thread_safety()
{
    this->abort_if_not_initialized("ft_unordered_map::disable_thread_safety");

    if (this->_mutex == ft_nullptr)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }

    int destroy_error;

    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    ft_unordered_map<Key, MappedType>::record_operation_error(destroy_error);
    return (destroy_error);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::is_thread_safe() const
{
    this->abort_if_not_initialized("ft_unordered_map::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::lock(bool *lock_acquired) const
{
    this->abort_if_not_initialized("ft_unordered_map::lock");

    int lock_error;

    lock_error = this->lock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
    return (lock_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::unlock(bool lock_acquired) const
{
    this->abort_if_not_initialized("ft_unordered_map::unlock");

    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    bool lock_acquired;
    int  lock_error;
    int  insert_error;
    int  unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::insert");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return ;
    }

    insert_error = this->insert_internal_unlocked(key, value);
    unlock_error = this->unlock_internal(lock_acquired);

    if (insert_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(insert_error);
        return ;
    }
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return ;
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator
ft_unordered_map<Key, MappedType>::find(const Key& key)
{
    bool    lock_acquired;
    int     lock_error;
    int     unlock_error;
    size_t  index;

    this->abort_if_not_initialized("ft_unordered_map::find");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (iterator(this->_data, this->_occupied, this->_capacity,
            this->_capacity));
    }

    index = this->find_index_unlocked(key);

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    if (index == this->_capacity)
        return (iterator(this->_data, this->_occupied, this->_capacity,
            this->_capacity));
    return (iterator(this->_data, this->_occupied, index, this->_capacity));
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator
ft_unordered_map<Key, MappedType>::find(const Key& key) const
{
    bool    lock_acquired;
    int     lock_error;
    int     unlock_error;
    size_t  index;

    this->abort_if_not_initialized("ft_unordered_map::find const");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (const_iterator(this->_data, this->_occupied, this->_capacity,
            this->_capacity));
    }

    index = this->find_index_unlocked(key);

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    if (index == this->_capacity)
        return (const_iterator(this->_data, this->_occupied, this->_capacity,
            this->_capacity));
    return (const_iterator(this->_data, this->_occupied, index, this->_capacity));
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::erase(const Key& key)
{
    bool    lock_acquired;
    int     lock_error;
    int     unlock_error;
    size_t  index;
    size_t  next_index;

    this->abort_if_not_initialized("ft_unordered_map::erase");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return ;
    }

    index = this->find_index_unlocked(key);
    if (index == this->_capacity)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return ;
    }

    ::destroy_at(&this->_data[index]);
    this->_occupied[index] = false;
    if (this->_size > 0)
        this->_size -= 1;

    next_index = (index + 1) % this->_capacity;
    while (this->_occupied[next_index])
    {
        ft_pair<Key, MappedType> moving_pair;
        int                      reinsert_error;

        moving_pair = this->_data[next_index];
        ::destroy_at(&this->_data[next_index]);
        this->_occupied[next_index] = false;
        if (this->_size > 0)
            this->_size -= 1;

        reinsert_error = this->insert_internal_unlocked(moving_pair.first, moving_pair.second);
        if (reinsert_error != FT_ERR_SUCCESS)
        {
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error == FT_ERR_SUCCESS)
                ft_unordered_map<Key, MappedType>::record_operation_error(reinsert_error);
            else
                ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
            return ;
        }
        next_index = (next_index + 1) % this->_capacity;
    }

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return ;
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::empty() const
{
    bool    lock_acquired;
    int     lock_error;
    bool    is_empty;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::empty");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (true);
    }

    is_empty = (this->_size == 0);

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return (is_empty);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::clear()
{
    bool    lock_acquired;
    int     lock_error;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::clear");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return ;
    }

    this->destroy_elements_unlocked();

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::size() const
{
    bool    lock_acquired;
    int     lock_error;
    size_t  current_size;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::size");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (0);
    }

    current_size = this->_size;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return (current_size);
}

template <typename Key, typename MappedType>
size_t ft_unordered_map<Key, MappedType>::bucket_count() const
{
    bool    lock_acquired;
    int     lock_error;
    size_t  bucket_total;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::bucket_count");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (0);
    }

    bucket_total = this->_capacity;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return (bucket_total);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::has_valid_storage() const
{
    bool    lock_acquired;
    int     lock_error;
    bool    valid_storage;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::has_valid_storage");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (false);
    }

    valid_storage = this->has_storage_unlocked();

    unlock_error = this->unlock_internal(lock_acquired);
    ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
    return (valid_storage);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::last_operation_error() noexcept
{
    return (ft_unordered_map<Key, MappedType>::operation_error_storage());
}

template <typename Key, typename MappedType>
const char *ft_unordered_map<Key, MappedType>::last_operation_error_str() noexcept
{
    return (ft_strerror(ft_unordered_map<Key, MappedType>::last_operation_error()));
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::operation_error_at(ft_size_t index) noexcept
{
    if (index == 0)
        return (ft_unordered_map<Key, MappedType>::operation_error_storage());
    return (FT_ERR_NOT_FOUND);
}

template <typename Key, typename MappedType>
const char *ft_unordered_map<Key, MappedType>::operation_error_str_at(
    ft_size_t index) noexcept
{
    return (ft_strerror(ft_unordered_map<Key, MappedType>::operation_error_at(index)));
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::pop_operation_errors() noexcept
{
    ft_unordered_map<Key, MappedType>::operation_error_storage() = FT_ERR_SUCCESS;
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::pop_oldest_operation_error() noexcept
{
    int error_code;

    error_code = ft_unordered_map<Key, MappedType>::operation_error_storage();
    ft_unordered_map<Key, MappedType>::operation_error_storage() = FT_ERR_SUCCESS;
    return (error_code);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::pop_newest_operation_error() noexcept
{
    int error_code;

    error_code = ft_unordered_map<Key, MappedType>::operation_error_storage();
    ft_unordered_map<Key, MappedType>::operation_error_storage() = FT_ERR_SUCCESS;
    return (error_code);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::begin()
{
    this->abort_if_not_initialized("ft_unordered_map::begin");

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (iterator(this->_data, this->_occupied, 0, this->_capacity));
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator ft_unordered_map<Key, MappedType>::end()
{
    this->abort_if_not_initialized("ft_unordered_map::end");

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (iterator(this->_data, this->_occupied, this->_capacity,
        this->_capacity));
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator
ft_unordered_map<Key, MappedType>::begin() const
{
    this->abort_if_not_initialized("ft_unordered_map::begin const");

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (const_iterator(this->_data, this->_occupied, 0, this->_capacity));
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator
ft_unordered_map<Key, MappedType>::end() const
{
    this->abort_if_not_initialized("ft_unordered_map::end const");

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (const_iterator(this->_data, this->_occupied, this->_capacity,
        this->_capacity));
}

template <typename Key, typename MappedType>
MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key)
{
    static MappedType  error_value = MappedType();
    bool                lock_acquired;
    int                 lock_error;
    size_t              key_index;
    int                 unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::at");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (error_value);
    }

    key_index = this->find_index_unlocked(key);
    if (key_index == this->_capacity)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error == FT_ERR_SUCCESS)
            ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_NOT_FOUND);
        else
            ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (error_value);
    }

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (error_value);
    }

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (this->_data[key_index].second);
}

template <typename Key, typename MappedType>
const MappedType& ft_unordered_map<Key, MappedType>::at(const Key& key) const
{
    static MappedType  error_value = MappedType();
    bool                lock_acquired;
    int                 lock_error;
    size_t              key_index;
    int                 unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::at const");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (error_value);
    }

    key_index = this->find_index_unlocked(key);
    if (key_index == this->_capacity)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error == FT_ERR_SUCCESS)
            ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_NOT_FOUND);
        else
            ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (error_value);
    }

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (error_value);
    }

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (this->_data[key_index].second);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::mapped_proxy::mapped_proxy()
    : _parent_map(ft_nullptr)
    , _mapped_value_pointer(ft_nullptr)
    , _last_error(FT_ERR_INVALID_STATE)
    , _is_valid(0)
{
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::mapped_proxy::mapped_proxy(
    ft_unordered_map<Key, MappedType> *parent_map,
    MappedType *mapped_value_pointer, int32_t error_code, int32_t is_valid)
    : _parent_map(parent_map)
    , _mapped_value_pointer(mapped_value_pointer)
    , _last_error(error_code)
    , _is_valid(is_valid)
{
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::mapped_proxy::~mapped_proxy()
{
    return ;
}

template <typename Key, typename MappedType>
MappedType *ft_unordered_map<Key, MappedType>::mapped_proxy::operator->()
{
    static MappedType error_value = MappedType();

    if (!this->_is_valid || this->_mapped_value_pointer == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        if (this->_parent_map != ft_nullptr)
            ft_unordered_map<Key, MappedType>::record_operation_error(
                this->_last_error);
        return (&error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    if (this->_parent_map != ft_nullptr)
        ft_unordered_map<Key, MappedType>::record_operation_error(
            this->_last_error);
    return (this->_mapped_value_pointer);
}

template <typename Key, typename MappedType>
MappedType &ft_unordered_map<Key, MappedType>::mapped_proxy::operator*()
{
    static MappedType error_value = MappedType();

    if (!this->_is_valid || this->_mapped_value_pointer == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        if (this->_parent_map != ft_nullptr)
            ft_unordered_map<Key, MappedType>::record_operation_error(
                this->_last_error);
        return (error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    if (this->_parent_map != ft_nullptr)
        ft_unordered_map<Key, MappedType>::record_operation_error(
            this->_last_error);
    return (*this->_mapped_value_pointer);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::mapped_proxy &
ft_unordered_map<Key, MappedType>::mapped_proxy::operator=(
    const MappedType& mapped_value)
{
    if (!this->_is_valid || this->_mapped_value_pointer == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        if (this->_parent_map != ft_nullptr)
            ft_unordered_map<Key, MappedType>::record_operation_error(
                this->_last_error);
        return (*this);
    }
    *this->_mapped_value_pointer = mapped_value;
    this->_last_error = FT_ERR_SUCCESS;
    if (this->_parent_map != ft_nullptr)
        ft_unordered_map<Key, MappedType>::record_operation_error(
            this->_last_error);
    return (*this);
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::mapped_proxy::operator MappedType() const
{
    if (!this->_is_valid || this->_mapped_value_pointer == ft_nullptr)
        return (MappedType());
    return (*this->_mapped_value_pointer);
}

template <typename Key, typename MappedType>
int32_t ft_unordered_map<Key, MappedType>::mapped_proxy::get_error() const
{
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int32_t ft_unordered_map<Key, MappedType>::mapped_proxy::is_valid() const
{
    return (this->_is_valid);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::mapped_proxy
ft_unordered_map<Key, MappedType>::operator[](const Key& key)
{
    bool    lock_acquired;
    int     lock_error;
    size_t  key_index;
    int     insert_error;
    int     unlock_error;

    this->abort_if_not_initialized("ft_unordered_map::operator[]");

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(lock_error);
        return (mapped_proxy(this, ft_nullptr, lock_error, 0));
    }

    key_index = this->find_index_unlocked(key);
    if (key_index == this->_capacity)
    {
        insert_error = this->insert_internal_unlocked(key, MappedType());
        if (insert_error != FT_ERR_SUCCESS)
        {
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error == FT_ERR_SUCCESS)
                ft_unordered_map<Key, MappedType>::record_operation_error(insert_error);
            else
                ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
            if (unlock_error == FT_ERR_SUCCESS)
                return (mapped_proxy(this, ft_nullptr, insert_error, 0));
            return (mapped_proxy(this, ft_nullptr, unlock_error, 0));
        }
        key_index = this->find_index_unlocked(key);
    }

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_unordered_map<Key, MappedType>::record_operation_error(unlock_error);
        return (mapped_proxy(this, ft_nullptr, unlock_error, 0));
    }

    ft_unordered_map<Key, MappedType>::record_operation_error(FT_ERR_SUCCESS);
    return (mapped_proxy(this, &this->_data[key_index].second, FT_ERR_SUCCESS,
        1));
}

// Iterator

template <typename Key, typename MappedType>
thread_local int32_t ft_unordered_map<Key, MappedType>::iterator::_last_error = FT_ERR_INVALID_STATE;

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::abort_lifecycle_error(
    const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_unordered_map::iterator lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::abort_if_not_initialized(
    const char *method_name) const
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::iterator()
    : _data(ft_nullptr)
    , _occupied(ft_nullptr)
    , _index(0)
    , _capacity(0)
    , _initialized_state(ft_unordered_map<Key, MappedType>::iterator::_state_uninitialized)
{
    this->_last_error = FT_ERR_INVALID_STATE;
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::iterator(
    ft_pair<Key, MappedType>* data, bool* occupied, size_t index, size_t capacity)
    : _data(ft_nullptr)
    , _occupied(ft_nullptr)
    , _index(0)
    , _capacity(0)
    , _initialized_state(ft_unordered_map<Key, MappedType>::iterator::_state_uninitialized)
{
    this->_last_error = FT_ERR_INVALID_STATE;
    (void)this->initialize(data, occupied, index, capacity);
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::iterator::~iterator()
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_uninitialized)
        this->abort_lifecycle_error("ft_unordered_map::iterator::~iterator",
            "destroyed while uninitialized");
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_initialized)
        (void)this->destroy();
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::initialize(
    ft_pair<Key, MappedType>* data, bool* occupied, size_t index, size_t capacity)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::iterator::initialize",
            "called while already initialized");
    this->_data = data;
    this->_occupied = occupied;
    this->_index = index;
    this->_capacity = capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::initialize(const iterator& other)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::iterator::initialize copy",
            "called while already initialized");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    this->_last_error = other._last_error;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::initialize(iterator&& other)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::iterator::initialize move",
            "called while already initialized");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    other._initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::destroy()
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_uninitialized)
        this->abort_lifecycle_error("ft_unordered_map::iterator::destroy",
            "called while object is uninitialized");
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::iterator::_state_destroyed)
        this->abort_lifecycle_error("ft_unordered_map::iterator::destroy",
            "called while object is already destroyed");
    this->_data = ft_nullptr;
    this->_occupied = ft_nullptr;
    this->_index = 0;
    this->_capacity = 0;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::move(iterator& other)
{
    this->abort_if_not_initialized("ft_unordered_map::iterator::move");
    other.abort_if_not_initialized("ft_unordered_map::iterator::move");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->advance_to_valid_index_unlocked();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    other._initialized_state
        = ft_unordered_map<Key, MappedType>::iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::iterator::get_error() const
{
    this->abort_if_not_initialized("ft_unordered_map::iterator::get_error");
    return (this->_last_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::iterator::advance_to_valid_index_unlocked()
{
    if (this->_occupied == ft_nullptr)
        return ;
    while (this->_index < this->_capacity)
    {
        if (this->_occupied[this->_index])
            return ;
        this->_index += 1;
    }
    return ;
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>& ft_unordered_map<Key, MappedType>::iterator::operator*() const
{
    static ft_pair<Key, MappedType> error_value;

    this->abort_if_not_initialized("ft_unordered_map::iterator::operator*");
    if (this->_data == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (error_value);
    }
    if (this->_occupied == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (error_value);
    }
    if (this->_index >= this->_capacity)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        return (error_value);
    }
    if (!this->_occupied[this->_index])
    {
        this->_last_error = FT_ERR_NOT_FOUND;
        return (error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_data[this->_index]);
}

template <typename Key, typename MappedType>
ft_pair<Key, MappedType>* ft_unordered_map<Key, MappedType>::iterator::operator->() const
{
    static ft_pair<Key, MappedType> error_value;

    this->abort_if_not_initialized("ft_unordered_map::iterator::operator->");
    if (this->_data == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (&error_value);
    }
    if (this->_occupied == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (&error_value);
    }
    if (this->_index >= this->_capacity)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        return (&error_value);
    }
    if (!this->_occupied[this->_index])
    {
        this->_last_error = FT_ERR_NOT_FOUND;
        return (&error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    return (&this->_data[this->_index]);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::iterator&
ft_unordered_map<Key, MappedType>::iterator::operator++()
{
    this->abort_if_not_initialized("ft_unordered_map::iterator::operator++");
    if (this->_index < this->_capacity)
        this->_index += 1;
    this->advance_to_valid_index_unlocked();
    this->_last_error = FT_ERR_SUCCESS;
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator==(const iterator& other) const
{
    this->abort_if_not_initialized("ft_unordered_map::iterator::operator==");
    other.abort_if_not_initialized("ft_unordered_map::iterator::operator==");
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_data == other._data
        && this->_occupied == other._occupied
        && this->_index == other._index
        && this->_capacity == other._capacity);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::iterator::operator!=(const iterator& other) const
{
    this->abort_if_not_initialized("ft_unordered_map::iterator::operator!=");
    other.abort_if_not_initialized("ft_unordered_map::iterator::operator!=");
    this->_last_error = FT_ERR_SUCCESS;
    return (!(*this == other));
}

template <typename Key, typename MappedType>
thread_local int32_t ft_unordered_map<Key, MappedType>::const_iterator::_last_error
    = FT_ERR_INVALID_STATE;

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::abort_lifecycle_error(
    const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_unordered_map::const_iterator lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::abort_if_not_initialized(
    const char *method_name) const
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator()
    : _data(ft_nullptr)
    , _occupied(ft_nullptr)
    , _index(0)
    , _capacity(0)
    , _initialized_state(ft_unordered_map<Key, MappedType>::const_iterator::_state_uninitialized)
{
    this->_last_error = FT_ERR_INVALID_STATE;
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::const_iterator(
    const ft_pair<Key, MappedType>* data, const bool* occupied,
    size_t index, size_t capacity)
    : _data(ft_nullptr)
    , _occupied(ft_nullptr)
    , _index(0)
    , _capacity(0)
    , _initialized_state(ft_unordered_map<Key, MappedType>::const_iterator::_state_uninitialized)
{
    this->_last_error = FT_ERR_INVALID_STATE;
    (void)this->initialize(data, occupied, index, capacity);
    return ;
}

template <typename Key, typename MappedType>
ft_unordered_map<Key, MappedType>::const_iterator::~const_iterator()
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_uninitialized)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::~const_iterator",
            "destroyed while uninitialized");
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized)
        (void)this->destroy();
    return ;
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::initialize(
    const ft_pair<Key, MappedType>* data, const bool* occupied,
    size_t index, size_t capacity)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::initialize",
            "called while already initialized");
    this->_data = data;
    this->_occupied = occupied;
    this->_index = index;
    this->_capacity = capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::initialize(
    const const_iterator& other)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::initialize copy",
            "called while already initialized");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    this->_last_error = other._last_error;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::initialize(
    const_iterator&& other)
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::initialize move",
            "called while already initialized");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_initialized;
    this->advance_to_valid_index_unlocked();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    other._initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::destroy()
{
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_uninitialized)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::destroy",
            "called while object is uninitialized");
    if (this->_initialized_state
        == ft_unordered_map<Key, MappedType>::const_iterator::_state_destroyed)
        this->abort_lifecycle_error("ft_unordered_map::const_iterator::destroy",
            "called while object is already destroyed");
    this->_data = ft_nullptr;
    this->_occupied = ft_nullptr;
    this->_index = 0;
    this->_capacity = 0;
    this->_initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::move(const_iterator& other)
{
    this->abort_if_not_initialized("ft_unordered_map::const_iterator::move");
    other.abort_if_not_initialized("ft_unordered_map::const_iterator::move");
    this->_data = other._data;
    this->_occupied = other._occupied;
    this->_index = other._index;
    this->_capacity = other._capacity;
    this->advance_to_valid_index_unlocked();
    other._data = ft_nullptr;
    other._occupied = ft_nullptr;
    other._index = 0;
    other._capacity = 0;
    other._initialized_state
        = ft_unordered_map<Key, MappedType>::const_iterator::_state_destroyed;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename Key, typename MappedType>
int ft_unordered_map<Key, MappedType>::const_iterator::get_error() const
{
    this->abort_if_not_initialized("ft_unordered_map::const_iterator::get_error");
    return (this->_last_error);
}

template <typename Key, typename MappedType>
void ft_unordered_map<Key, MappedType>::const_iterator::advance_to_valid_index_unlocked()
{
    if (this->_occupied == ft_nullptr)
        return ;
    while (this->_index < this->_capacity)
    {
        if (this->_occupied[this->_index])
            return ;
        this->_index += 1;
    }
    return ;
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>&
ft_unordered_map<Key, MappedType>::const_iterator::operator*() const
{
    static ft_pair<Key, MappedType> error_value;

    this->abort_if_not_initialized("ft_unordered_map::const_iterator::operator*");
    if (this->_data == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (error_value);
    }
    if (this->_occupied == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (error_value);
    }
    if (this->_index >= this->_capacity)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        return (error_value);
    }
    if (!this->_occupied[this->_index])
    {
        this->_last_error = FT_ERR_NOT_FOUND;
        return (error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_data[this->_index]);
}

template <typename Key, typename MappedType>
const ft_pair<Key, MappedType>*
ft_unordered_map<Key, MappedType>::const_iterator::operator->() const
{
    static ft_pair<Key, MappedType> error_value;

    this->abort_if_not_initialized("ft_unordered_map::const_iterator::operator->");
    if (this->_data == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (&error_value);
    }
    if (this->_occupied == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_POINTER;
        return (&error_value);
    }
    if (this->_index >= this->_capacity)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        return (&error_value);
    }
    if (!this->_occupied[this->_index])
    {
        this->_last_error = FT_ERR_NOT_FOUND;
        return (&error_value);
    }
    this->_last_error = FT_ERR_SUCCESS;
    return (&this->_data[this->_index]);
}

template <typename Key, typename MappedType>
typename ft_unordered_map<Key, MappedType>::const_iterator&
ft_unordered_map<Key, MappedType>::const_iterator::operator++()
{
    this->abort_if_not_initialized("ft_unordered_map::const_iterator::operator++");
    if (this->_index < this->_capacity)
        this->_index += 1;
    this->advance_to_valid_index_unlocked();
    this->_last_error = FT_ERR_SUCCESS;
    return (*this);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator==(
    const const_iterator& other) const
{
    this->abort_if_not_initialized("ft_unordered_map::const_iterator::operator==");
    other.abort_if_not_initialized("ft_unordered_map::const_iterator::operator==");
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_data == other._data
        && this->_occupied == other._occupied
        && this->_index == other._index
        && this->_capacity == other._capacity);
}

template <typename Key, typename MappedType>
bool ft_unordered_map<Key, MappedType>::const_iterator::operator!=(
    const const_iterator& other) const
{
    this->abort_if_not_initialized("ft_unordered_map::const_iterator::operator!=");
    other.abort_if_not_initialized("ft_unordered_map::const_iterator::operator!=");
    this->_last_error = FT_ERR_SUCCESS;
    return (!(*this == other));
}

#ifdef LIBFT_TEST_BUILD
template <typename Key, typename MappedType>
pt_recursive_mutex *ft_unordered_map<Key, MappedType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template <typename Key, typename MappedType>
using ft_unord_map = ft_unordered_map<Key, MappedType>;

#endif
