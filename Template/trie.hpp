#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "unordered_map.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ValueType>
class ft_trie
{
    private:
        struct node_value
        {
            ft_size_t  _key_length;
            int32_t    _unset_value;
            ValueType  *_value_pointer;
        };

        node_value                                  *_data;
        ft_unordered_map<char, ft_trie<ValueType>*> _children;
        mutable pt_recursive_mutex                  *_mutex;
        uint8_t                                     _initialised_state;
        static thread_local uint32_t                _last_error;

        static uint32_t set_error(uint32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;
        ft_size_t key_length(const char *key) const;
        void destroy_children_unlocked();
        int32_t insert_helper(const char *key, int32_t unset_value,
            ValueType *value_pointer);

    public:
        ft_trie();
        ft_trie(const ft_trie<ValueType> &other);
        ft_trie(ft_trie<ValueType> &&other);
        ~ft_trie();

        ft_trie &operator=(const ft_trie &other) = delete;
        ft_trie &operator=(ft_trie &&other) = delete;

        int32_t initialize();
        int32_t destroy();
        uint32_t move(ft_trie<ValueType> &other);

        int32_t insert(const char *key, ValueType *value_pointer,
            int32_t unset_value = 0);
        const node_value *search(const char *key) const;

        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;

        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;

        uint32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

template <typename ValueType>
uint32_t ft_trie<ValueType>::set_error(uint32_t error_code) noexcept
{
    _last_error = error_code;
    return (error_code);
}

template <typename ValueType>
int32_t ft_trie<ValueType>::lock_internal(ft_bool *lock_acquired) const
{
    int32_t lock_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (set_error(lock_result));
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_trie<ValueType>::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ft_size_t ft_trie<ValueType>::key_length(const char *key) const
{
    ft_size_t index;

    index = 0;
    while (key[index] != '\0')
        ++index;
    return (index);
}

template <typename ValueType>
void ft_trie<ValueType>::destroy_children_unlocked()
{
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
            delete child_iterator->second;
        ++child_iterator;
    }
    this->_children.clear();
    return ;
}

template <typename ValueType>
int32_t ft_trie<ValueType>::insert_helper(const char *key, int32_t unset_value,
    ValueType *value_pointer)
{
    ft_trie<ValueType> *current_node;
    const char *key_iterator;
    ft_size_t length_value;

    if (key == ft_nullptr)
        return (set_error(FT_ERR_INVALID_ARGUMENT));
    length_value = this->key_length(key);
    current_node = this;
    key_iterator = key;
    while (*key_iterator != '\0')
    {
        char character;
        ft_trie<ValueType> *new_child;
        int32_t child_result;

        character = *key_iterator;
        if (current_node->_children[character] == ft_nullptr)
        {
            new_child = new (std::nothrow) ft_trie<ValueType>();
            if (new_child == ft_nullptr)
                return (set_error(FT_ERR_NO_MEMORY));
            if (new_child->initialize() != FT_ERR_SUCCESS)
            {
                delete new_child;
                return (set_error(FT_ERR_INVALID_STATE));
            }
            current_node->_children[character] = new_child;
            if (this->_mutex != ft_nullptr)
            {
                child_result = new_child->enable_thread_safety();
                if (child_result != FT_ERR_SUCCESS)
                {
                    current_node->_children[character] = ft_nullptr;
                    delete new_child;
                    return (set_error(child_result));
                }
            }
        }
        current_node = current_node->_children[character];
        ++key_iterator;
    }
    if (current_node->_data == ft_nullptr)
    {
        current_node->_data = static_cast<node_value*>(cma_malloc(sizeof(node_value)));
        if (current_node->_data == ft_nullptr)
            return (set_error(FT_ERR_NO_MEMORY));
    }
    current_node->_data->_unset_value = unset_value;
    current_node->_data->_key_length = length_value;
    current_node->_data->_value_pointer = value_pointer;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
ft_trie<ValueType>::ft_trie()
    : _data(ft_nullptr), _children(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::ft_trie(const ft_trie<ValueType> &other)
    : _data(ft_nullptr), _children(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    ft_bool lock_acquired;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_trie::ft_trie(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._data != ft_nullptr)
    {
        this->_data = static_cast<node_value*>(cma_malloc(sizeof(node_value)));
        if (this->_data == ft_nullptr)
        {
            (void)other.unlock_internal(lock_acquired);
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
        *this->_data = *other._data;
    }
    typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_iterator(
        other._children.begin());
    typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_end(
        other._children.end());
    while (child_iterator != child_end)
    {
        ft_trie<ValueType> *new_child;

        new_child = new (std::nothrow) ft_trie<ValueType>(*child_iterator->second);
        if (new_child == ft_nullptr
            || new_child->_initialised_state == FT_CLASS_STATE_DESTROYED)
        {
            if (new_child != ft_nullptr)
                delete new_child;
            (void)other.unlock_internal(lock_acquired);
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
        this->_children[child_iterator->first] = new_child;
        ++child_iterator;
    }
    (void)other.unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::ft_trie(ft_trie<ValueType> &&other)
    : _data(ft_nullptr), _children(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_trie::ft_trie(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->move(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::~ft_trie()
{
    uint32_t previous_error;

    previous_error = _last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (this->_mutex != ft_nullptr)
        (void)this->disable_thread_safety();
    (void)set_error(previous_error);
    return ;
}

template <typename ValueType>
int32_t ft_trie<ValueType>::initialize()
{
    int32_t children_result;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_trie::initialize", "called while object is already initialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_data = ft_nullptr;
    children_result = this->_children.initialize();
    if (children_result != FT_ERR_SUCCESS)
    {
        (void)this->_children.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(children_result));
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t ft_trie<ValueType>::destroy()
{
    ft_bool lock_acquired;
    int32_t lock_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (set_error(FT_ERR_SUCCESS));
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (set_error(lock_error));
    this->destroy_children_unlocked();
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    (void)this->unlock_internal(lock_acquired);
    (void)this->_children.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
uint32_t ft_trie<ValueType>::move(ft_trie<ValueType> &other)
{
    int32_t destroy_result;

    if (this == &other)
        return (set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_trie::move",
            "source object is uninitialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (set_error(destroy_result));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_data = ft_nullptr;
        (void)this->_children.destroy();
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_SUCCESS));
    }
    this->_data = other._data;
    this->_children = ft_move(other._children);
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._data = ft_nullptr;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t ft_trie<ValueType>::insert(const char *key, ValueType *value_pointer,
    int32_t unset_value)
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t insert_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::insert");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (set_error(lock_error));
    insert_result = this->insert_helper(key, unset_value, value_pointer);
    (void)this->unlock_internal(lock_acquired);
    return (set_error(insert_result));
}

template <typename ValueType>
const typename ft_trie<ValueType>::node_value *ft_trie<ValueType>::search(
    const char *key) const
{
    const ft_trie<ValueType> *current_node;
    const char *key_iterator;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::search");
    if (key == ft_nullptr)
    {
        set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        set_error(lock_error);
        return (ft_nullptr);
    }
    current_node = this;
    key_iterator = key;
    while (*key_iterator != '\0')
    {
        typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_iterator(
            current_node->_children.find(*key_iterator));
        if (child_iterator == current_node->_children.end()
            || child_iterator->second == ft_nullptr)
        {
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_NOT_FOUND);
            return (ft_nullptr);
        }
        current_node = child_iterator->second;
        ++key_iterator;
    }
    (void)this->unlock_internal(lock_acquired);
    set_error(FT_ERR_SUCCESS);
    return (current_node->_data);
}

template <typename ValueType>
int32_t ft_trie<ValueType>::enable_thread_safety()
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (set_error(FT_ERR_NO_MEMORY));
    initialize_result = new_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (set_error(initialize_result));
    }
    this->_mutex = new_mutex;
    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
        {
            initialize_result = child_iterator->second->enable_thread_safety();
            if (initialize_result != FT_ERR_SUCCESS)
                return (set_error(initialize_result));
        }
        ++child_iterator;
    }
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t ft_trie<ValueType>::disable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        && this->_initialised_state != FT_CLASS_STATE_DESTROYED)
        return (set_error(FT_ERR_INVALID_STATE));
    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
            (void)child_iterator->second->disable_thread_safety();
        ++child_iterator;
    }
    mutex_pointer = this->_mutex;
    if (mutex_pointer == ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    this->_mutex = ft_nullptr;
    destroy_result = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_result != FT_ERR_SUCCESS)
        return (set_error(destroy_result));
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
ft_bool ft_trie<ValueType>::is_thread_safe() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::is_thread_safe");
    set_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

template <typename ValueType>
int32_t ft_trie<ValueType>::lock(ft_bool *lock_acquired) const
{
    int32_t lock_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::lock");
    lock_result = this->lock_internal(lock_acquired);
    return (set_error(lock_result));
}

template <typename ValueType>
void ft_trie<ValueType>::unlock(ft_bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    set_error(FT_ERR_SUCCESS);
    return ;
}

template <typename ValueType>
uint32_t ft_trie<ValueType>::get_error() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_trie::get_error");
    return (_last_error);
}

template <typename ValueType>
const char *ft_trie<ValueType>::get_error_str() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_trie::get_error_str");
    return (ft_strerror(_last_error));
}

template <typename ValueType>
thread_local uint32_t ft_trie<ValueType>::_last_error = FT_ERR_SUCCESS;

#endif
