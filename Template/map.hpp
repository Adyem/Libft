#ifndef FT_MAP_HPP
#define FT_MAP_HPP

#include "pair.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

template <typename Key, typename MappedType>
class ft_map
{
    private:
        Pair<Key, MappedType>*      _data;
        size_t                      _capacity;
        size_t                      _size;
        mutable pt_recursive_mutex* _mutex;
        uint8_t                     _state;
        size_t                      _initial_capacity;

        static const uint8_t        _state_uninitialized = 0;
        static const uint8_t        _state_destroyed = 1;
        static const uint8_t        _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;

        bool    ensure_capacity(size_t desired_capacity);
        size_t  find_index(const Key& key) const;
        void    destroy_all_unlocked();
        Pair<Key, MappedType>*  end_pointer_unlocked();
        const Pair<Key, MappedType>* end_pointer_unlocked() const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     lock_pair(const ft_map &other, bool *lock_this, bool *lock_other) const;
        void    unlock_pair(const ft_map &other, bool lock_this, bool lock_other) const;
        template <typename Type, typename = void>
        struct has_initialize_move
            : std::false_type
        {
        };
        template <typename Type>
        struct has_initialize_move<Type,
            std::void_t<decltype(std::declval<Type&>().initialize(std::declval<Type&&>()))> >
            : std::true_type
        {
        };
        template <typename Type, typename = void>
        struct has_initialize_copy
            : std::false_type
        {
        };
        template <typename Type>
        struct has_initialize_copy<Type,
            std::void_t<decltype(std::declval<Type&>().initialize(std::declval<const Type&>()))> >
            : std::true_type
        {
        };
        template <typename Type, typename = void>
        struct has_destroy
            : std::false_type
        {
        };
        template <typename Type>
        struct has_destroy<Type, std::void_t<decltype(std::declval<Type&>().destroy())> >
            : std::true_type
        {
        };

    public:
        ft_map(size_t initial_capacity = 10);
        ~ft_map();

        ft_map(const ft_map& other) = delete;
        ft_map& operator=(const ft_map& other);
        ft_map(ft_map&& other) = delete;
        ft_map& operator=(ft_map&& other);

        int     initialize();
        int     destroy();

        int     copy_from(const ft_map& other);
        int     move_from(ft_map& other);

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
        int         disable_thread_safety();
        bool        is_thread_safe() const;
        int         lock(bool *lock_acquired) const;
        int         unlock(bool lock_acquired) const;

        int8_t      compare(const ft_map& other) const;

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
    #endif
};

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::ft_map(size_t initial_capacity)
    : _data(ft_nullptr)
    , _capacity(initial_capacity)
    , _size(0)
    , _mutex(ft_nullptr)
    , _state(ft_map<Key, MappedType>::_state_uninitialized)
    , _initial_capacity(initial_capacity)
{
    return ;
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(
    const ft_map<Key, MappedType>& other)
{
    if (this == &other)
        return (*this);
    if (other._state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map::operator=(const ft_map&)",
            "source object is not initialized");
        return (*this);
    }
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
        (void)this->initialize();
    (void)this->copy_from(other);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>& ft_map<Key, MappedType>::operator=(
    ft_map<Key, MappedType>&& other)
{
    if (this == &other)
        return (*this);
    if (other._state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map::operator=(ft_map&&)",
            "source object is not initialized");
        return (*this);
    }
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
        (void)this->initialize();
    (void)this->move_from(other);
    return (*this);
}

template <typename Key, typename MappedType>
ft_map<Key, MappedType>::~ft_map()
{
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->_state = ft_map<Key, MappedType>::_state_destroyed;
        return ;
    }
    (void)this->destroy();
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::abort_lifecycle_error(const char *method_name,
        const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_map lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::abort_if_not_initialized(const char *method_name) const
{
    if (this->_state == ft_map<Key, MappedType>::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::initialize()
{
    if (this->_state == ft_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_capacity = this->_initial_capacity;
    this->_size = 0;
    if (this->_capacity > 0)
    {
        void *raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * this->_capacity);

        if (raw_memory == ft_nullptr)
        {
            this->_capacity = 0;
            this->_state = ft_map<Key, MappedType>::_state_destroyed;
            return (FT_ERR_NO_MEMORY);
        }
        this->_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    }
    int thread_error = this->enable_thread_safety();
    if (thread_error != FT_ERR_SUCCESS)
    {
        if (this->_data != ft_nullptr)
        {
            cma_free(this->_data);
            this->_data = ft_nullptr;
        }
        this->_capacity = 0;
        this->_state = ft_map<Key, MappedType>::_state_destroyed;
        return (thread_error);
    }
    this->_state = ft_map<Key, MappedType>::_state_initialized;
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::destroy()
{
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->_state = ft_map<Key, MappedType>::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    this->clear();
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_capacity = 0;
    this->_size = 0;
    (void)this->disable_thread_safety();
    this->_state = ft_map<Key, MappedType>::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::copy_from(const ft_map<Key, MappedType>& other)
{
    if (other._state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map::copy_from",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->_initial_capacity = other._capacity;
        int init_error = this->initialize();
        if (init_error != FT_ERR_SUCCESS)
            return (init_error);
    }
    bool lock_this = false;
    bool lock_other = false;
    int lock_error = this->lock_pair(other, &lock_this, &lock_other);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->destroy_all_unlocked();
    if (this->_capacity < other._size)
    {
        size_t next_capacity = other._size;
        if (!this->ensure_capacity(next_capacity))
        {
            this->unlock_pair(other, lock_this, lock_other);
            return (FT_ERR_NO_MEMORY);
        }
    }
    size_t index = 0;
    while (index < other._size)
    {
        construct_at(&this->_data[index], other._data[index]);
        index++;
    }
    this->_size = other._size;
    this->unlock_pair(other, lock_this, lock_other);
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::move_from(ft_map<Key, MappedType>& other)
{
    if (other._state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_map::move_from",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_state != ft_map<Key, MappedType>::_state_initialized)
    {
        this->_initial_capacity = 0;
        int init_error = this->initialize();
        if (init_error != FT_ERR_SUCCESS)
            return (init_error);
    }
    bool lock_this = false;
    bool lock_other = false;
    int lock_error = this->lock_pair(other, &lock_this, &lock_other);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->destroy_all_unlocked();
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_data = other._data;
    this->_capacity = other._capacity;
    this->_size = other._size;
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    this->unlock_pair(other, lock_this, lock_other);
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::ensure_capacity(size_t desired_capacity)
{
    this->abort_if_not_initialized("ft_map::ensure_capacity");
    if (desired_capacity <= this->_capacity)
        return (true);
    void *raw_memory = cma_malloc(sizeof(Pair<Key, MappedType>) * desired_capacity);

    if (raw_memory == ft_nullptr)
        return (false);
    Pair<Key, MappedType> *new_data = static_cast<Pair<Key, MappedType> *>(raw_memory);
    size_t index = 0;

    while (index < this->_size)
    {
        construct_at(&new_data[index]);
        new_data[index].key = this->_data[index].key;
        if constexpr (std::is_pointer<MappedType>::value)
            new_data[index].value = this->_data[index].value;
        else if constexpr (has_initialize_move<MappedType>::value)
        {
            int initialize_error;

            initialize_error = new_data[index].value.initialize(this->_data[index].value);
            if (initialize_error != FT_ERR_SUCCESS)
            {
                size_t cleanup_index = 0;
                while (cleanup_index <= index)
                {
                    destroy_at(&new_data[cleanup_index]);
                    cleanup_index++;
                }
                cma_free(new_data);
                return (false);
            }
        }
        else
            new_data[index].value = this->_data[index].value;
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
    this->abort_if_not_initialized("ft_map::find_index");
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
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end_pointer_unlocked() const
{
    return (this->_data + this->_size);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock_internal(bool *lock_acquired) const
{
    this->abort_if_not_initialized("ft_map::lock_internal");
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
int ft_map<Key, MappedType>::unlock_internal(bool lock_acquired) const
{
    this->abort_if_not_initialized("ft_map::unlock_internal");
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock_pair(const ft_map<Key, MappedType> &other,
    bool *lock_this, bool *lock_other) const
{
    if (lock_this)
        *lock_this = false;
    if (lock_other)
        *lock_other = false;
    if (this == &other)
        return (FT_ERR_SUCCESS);
    const ft_map<Key, MappedType> *first = this;
    const ft_map<Key, MappedType> *second = &other;
    if (first->_mutex > second->_mutex)
    {
        first = &other;
        second = this;
    }
    bool first_locked = false;
    int first_error = first->lock_internal(&first_locked);
    if (first_error != FT_ERR_SUCCESS)
        return (first_error);
    bool second_locked = false;
    int second_error = second->lock_internal(&second_locked);
    if (second_error != FT_ERR_SUCCESS)
    {
        (void)first->unlock_internal(first_locked);
        return (second_error);
    }
    if (lock_this)
    {
        if (first == this)
            *lock_this = first_locked;
        else
            *lock_this = second_locked;
    }
    if (lock_other)
    {
        if (first == &other)
            *lock_other = first_locked;
        else
            *lock_other = second_locked;
    }
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::unlock_pair(const ft_map<Key, MappedType> &other,
    bool lock_this, bool lock_other) const
{
    if (this == &other)
        return ;
    (void)other.unlock_internal(lock_other);
    (void)this->unlock_internal(lock_this);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, const MappedType& value)
{
    this->abort_if_not_initialized("ft_map::insert");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return ;
    size_t index = this->find_index(key);

    if (index != this->_size)
    {
        if constexpr (ft_map<Key, MappedType>::template has_destroy<MappedType>::value
            && ft_map<Key, MappedType>::template has_initialize_copy<MappedType>::value)
        {
            int destroy_error = this->_data[index].value.destroy();
            if (destroy_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            int initialize_error = this->_data[index].value.initialize(value);
            if (initialize_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
        }
        else if constexpr (std::is_copy_assignable<MappedType>::value)
            this->_data[index].value = value;
        (void)this->unlock_internal(lock_acquired);
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
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = incremented_capacity;
        }
    }
    if (!this->ensure_capacity(next_capacity))
    {
        (void)this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size]);
    this->_data[this->_size].key = key;
    if constexpr (ft_map<Key, MappedType>::template has_initialize_copy<MappedType>::value)
    {
        int initialize_error = this->_data[this->_size].value.initialize(value);
        if (initialize_error != FT_ERR_SUCCESS)
        {
            destroy_at(&this->_data[this->_size]);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }
    }
    else if constexpr (std::is_copy_assignable<MappedType>::value)
        this->_data[this->_size].value = value;
    this->_size += 1;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::insert(const Key& key, MappedType&& value)
{
    this->abort_if_not_initialized("ft_map::insert_move");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return ;
    size_t index = this->find_index(key);

    if (index != this->_size)
    {
        if constexpr (ft_map<Key, MappedType>::template has_destroy<MappedType>::value
            && ft_map<Key, MappedType>::template has_initialize_move<MappedType>::value)
        {
            int destroy_error = this->_data[index].value.destroy();
            if (destroy_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            int initialize_error = this->_data[index].value.initialize(ft_move(value));
            if (initialize_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
        }
        else if constexpr (std::is_move_assignable<MappedType>::value)
            this->_data[index].value = ft_move(value);
        (void)this->unlock_internal(lock_acquired);
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
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            next_capacity = incremented_capacity;
        }
    }
    if (!this->ensure_capacity(next_capacity))
    {
        (void)this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size]);
    this->_data[this->_size].key = key;
    if constexpr (ft_map<Key, MappedType>::template has_initialize_move<MappedType>::value)
    {
        int initialize_error = this->_data[this->_size].value.initialize(ft_move(value));
        if (initialize_error != FT_ERR_SUCCESS)
        {
            destroy_at(&this->_data[this->_size]);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }
    }
    else if constexpr (std::is_move_assignable<MappedType>::value)
        this->_data[this->_size].value = ft_move(value);
    this->_size += 1;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key)
{
    this->abort_if_not_initialized("ft_map::find");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);
    Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESS)
        return (this->end_pointer_unlocked());
    size_t index = this->find_index(key);

    if (index == this->_size)
        result = this->end_pointer_unlocked();
    else
        result = &this->_data[index];
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::find(const Key& key) const
{
    this->abort_if_not_initialized("ft_map::find_const");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    const Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESS)
        return (this->end_pointer_unlocked());
    size_t index = this->find_index(key);

    if (index == this->_size)
        result = this->end_pointer_unlocked();
    else
        result = &this->_data[index];
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::remove(const Key& key)
{
    this->abort_if_not_initialized("ft_map::remove");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return ;
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        (void)this->unlock_internal(lock_acquired);
        return ;
    }
    size_t move_index = index;

    while (move_index + 1 < this->_size)
    {
        this->_data[move_index].key = this->_data[move_index + 1].key;
        if constexpr (ft_map<Key, MappedType>::template has_destroy<MappedType>::value
            && ft_map<Key, MappedType>::template has_initialize_copy<MappedType>::value)
        {
            int destroy_error = this->_data[move_index].value.destroy();
            if (destroy_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            int initialize_error = this->_data[move_index].value.initialize(this->_data[move_index + 1].value);
            if (initialize_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
        }
        else if constexpr (std::is_copy_assignable<MappedType>::value)
            this->_data[move_index].value = this->_data[move_index + 1].value;
        move_index++;
    }
    if (this->_size > 0)
    {
        destroy_at(&this->_data[this->_size - 1]);
        this->_size -= 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::empty() const
{
    this->abort_if_not_initialized("ft_map::empty");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    bool    is_empty;

    if (lock_error != FT_ERR_SUCCESS)
        return (true);
    is_empty = (this->_size == 0);
    (void)this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename Key, typename MappedType>
void ft_map<Key, MappedType>::clear()
{
    this->abort_if_not_initialized("ft_map::clear");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->destroy_all_unlocked();
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::size() const
{
    this->abort_if_not_initialized("ft_map::size");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    size_t current_size;

    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    current_size = this->_size;
    (void)this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename Key, typename MappedType>
size_t ft_map<Key, MappedType>::capacity() const
{
    this->abort_if_not_initialized("ft_map::capacity");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    size_t current_capacity;

    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    current_capacity = this->_capacity;
    (void)this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename Key, typename MappedType>
Pair<Key, MappedType>* ft_map<Key, MappedType>::end()
{
    this->abort_if_not_initialized("ft_map::end");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);
    Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESS)
        return (this->end_pointer_unlocked());
    result = this->end_pointer_unlocked();
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const Pair<Key, MappedType>* ft_map<Key, MappedType>::end() const
{
    this->abort_if_not_initialized("ft_map::end_const");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);
    const Pair<Key, MappedType>* result;

    if (lock_error != FT_ERR_SUCCESS)
        return (this->end_pointer_unlocked());
    result = this->end_pointer_unlocked();
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
MappedType& ft_map<Key, MappedType>::at(const Key& key)
{
    this->abort_if_not_initialized("ft_map::at");
    bool    lock_acquired = false;
    int     lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return (this->_data[0].value);
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        (void)this->unlock_internal(lock_acquired);
        return (this->_data[0].value);
    }
    MappedType& result = this->_data[index].value;
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
const MappedType& ft_map<Key, MappedType>::at(const Key& key) const
{
    this->abort_if_not_initialized("ft_map::at_const");
    bool    lock_acquired = false;
    int     lock_error = const_cast<ft_map<Key, MappedType> *>(this)->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return (this->_data[0].value);
    size_t index = this->find_index(key);

    if (index == this->_size)
    {
        (void)this->unlock_internal(lock_acquired);
        return (this->_data[0].value);
    }
    const MappedType& result = this->_data[index].value;
    (void)this->unlock_internal(lock_acquired);
    return (result);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    this->_mutex = new (std::nothrow) pt_recursive_mutex();
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int init_error = this->_mutex->initialize();
    if (init_error != FT_ERR_SUCCESS)
    {
        delete this->_mutex;
        this->_mutex = ft_nullptr;
        return (init_error);
    }
    return (FT_ERR_SUCCESS);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::disable_thread_safety()
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

template <typename Key, typename MappedType>
bool ft_map<Key, MappedType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::lock(bool *lock_acquired) const
{
    this->abort_if_not_initialized("ft_map::lock");
    return (this->lock_internal(lock_acquired));
}

template <typename Key, typename MappedType>
int ft_map<Key, MappedType>::unlock(bool lock_acquired) const
{
    this->abort_if_not_initialized("ft_map::unlock");
    return (this->unlock_internal(lock_acquired));
}

template <typename Key, typename MappedType>
int8_t ft_map<Key, MappedType>::compare(const ft_map<Key, MappedType>& other) const
{
    if (this == &other)
        return (0);
    this->abort_if_not_initialized("ft_map::compare");
    if (other._state != ft_map<Key, MappedType>::_state_initialized)
    {
        const_cast<ft_map<Key, MappedType> *>(this)->abort_lifecycle_error(
            "ft_map::compare", "source object is not initialized");
        return (-1);
    }
    bool lock_this = false;
    bool lock_other = false;
    int lock_error = this->lock_pair(other, &lock_this, &lock_other);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_size != other._size)
    {
        this->unlock_pair(other, lock_this, lock_other);
        return (1);
    }
    size_t index = 0;
    while (index < this->_size)
    {
        const Key &current_key = this->_data[index].key;
        size_t other_index = other.find_index(current_key);
        if (other_index == other._size)
        {
            this->unlock_pair(other, lock_this, lock_other);
            return (1);
        }
        if (!(this->_data[index].value == other._data[other_index].value))
        {
            this->unlock_pair(other, lock_this, lock_other);
            return (1);
        }
        index++;
    }
    this->unlock_pair(other, lock_this, lock_other);
    return (0);
}

#ifdef LIBFT_TEST_BUILD

template <typename Key, typename MappedType>
pt_recursive_mutex* ft_map<Key, MappedType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}

#endif

#endif
