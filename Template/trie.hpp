#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "unordered_map.hpp"
#include <cstddef>
#include <new>

template <typename ValueType>
class ft_trie
{
    private:
        struct node_value
        {
            size_t      _key_length;
            int         _unset_value;
            ValueType  *_value_pointer;
        };

        node_value                                      *_data;
        ft_unordered_map<char, ft_trie<ValueType>*>     _children;
        mutable int                                      _error_code;
        mutable int                                      _last_error;
        mutable pt_mutex                                *_state_mutex;
        bool                                             _thread_safe_enabled;

        int insert_helper(const char *key, int unset_value, ValueType *value_pointer);
        void set_error(int error) const;
        void set_success_preserve_errno(int entry_errno) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        ft_trie();
        ~ft_trie();

        ft_trie(const ft_trie &other) = delete;
        ft_trie &operator=(const ft_trie &other) = delete;
        ft_trie(ft_trie &&other) = delete;
        ft_trie &operator=(ft_trie &&other) = delete;

        int insert(const char *key, ValueType *value_pointer, int unset_value = 0);
        const node_value *search(const char *key) const;
        int get_error() const;
        const char *get_error_str() const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename ValueType>
ft_trie<ValueType>::ft_trie()
        : _data(ft_nullptr), _children(), _error_code(FT_ERR_SUCCESSS),
        _last_error(FT_ERR_SUCCESSS), _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::~ft_trie()
{
    this->disable_thread_safety();
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());
    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
            delete child_iterator->second;
        ++child_iterator;
    }
    this->_children.clear();
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
int ft_trie<ValueType>::insert_helper(const char *key, int unset_value, ValueType *value_pointer)
{
    ft_trie<ValueType> *current_node;
    const char *key_iterator;
    size_t key_length;

    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_error_code);
        return (1);
    }
    if (key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (1);
    }
    key_length = ft_strlen_size_t(key);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (1);
    }
    current_node = this;
    key_iterator = key;
    while (*key_iterator)
    {
        char character;

        character = *key_iterator;
        if (current_node->_children[character] == ft_nullptr)
        {
            ft_trie<ValueType> *new_child;

            new_child = new (std::nothrow) ft_trie<ValueType>();
            if (new_child == ft_nullptr)
            {
                this->set_error(FT_ERR_NO_MEMORY);
                return (1);
            }
            current_node->_children[character] = new_child;
            if (current_node->_thread_safe_enabled && current_node->_state_mutex != ft_nullptr)
            {
                if (new_child->enable_thread_safety() != 0)
                {
                    int child_error;

                    child_error = new_child->get_error();
                    current_node->_children[character] = ft_nullptr;
                    delete new_child;
                    this->set_error(child_error);
                    return (1);
                }
            }
        }
        current_node = current_node->_children[character];
        key_iterator = key_iterator + 1;
    }
    if (current_node->_data == ft_nullptr)
    {
        current_node->_data = static_cast<node_value *>(cma_malloc(sizeof(node_value)));
        if (current_node->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (1);
        }
    }
    ft_bzero(current_node->_data, sizeof(node_value));
    current_node->_data->_unset_value = unset_value;
    current_node->_data->_key_length = key_length;
    current_node->_data->_value_pointer = value_pointer;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ValueType>
int ft_trie<ValueType>::insert(const char *key, ValueType *value_pointer, int unset_value)
{
    int result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (1);
    }
    result = this->insert_helper(key, unset_value, value_pointer);
    this->unlock_internal(lock_acquired);
    if (result == 0)
        this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
const typename ft_trie<ValueType>::node_value *ft_trie<ValueType>::search(const char *key) const
{
    const ft_trie<ValueType> *current_node;
    const char *key_iterator;
    bool lock_acquired;
    bool key_found;
    const node_value *result;

    if (key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    current_node = this;
    key_iterator = key;
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (ft_nullptr);
    }
    key_found = true;
    while (*key_iterator && key_found)
    {
        typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_iterator(current_node->_children.find(*key_iterator));
        if (child_iterator == current_node->_children.end())
        {
            this->set_error(FT_ERR_SUCCESSS);
            key_found = false;
            break;
        }
        if (child_iterator->second == ft_nullptr)
        {
            this->set_error(FT_ERR_SUCCESSS);
            key_found = false;
            break;
        }
        current_node = child_iterator->second;
        key_iterator = key_iterator + 1;
    }
    if (key_found && current_node->_data != ft_nullptr)
        result = current_node->_data;
    else
        result = ft_nullptr;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ValueType>
int ft_trie<ValueType>::get_error() const
{
    return (this->_error_code);
}

template <typename ValueType>
const char *ft_trie<ValueType>::get_error_str() const
{
    return (ft_strerror(this->_last_error));
}

template <typename ValueType>
void ft_trie<ValueType>::set_error(int error) const
{
    this->_last_error = error;
    ft_errno = error;
    if (error == FT_ERR_SUCCESSS)
    {
        this->_error_code = FT_ERR_SUCCESSS;
        return ;
    }
    this->_error_code = 1;
    return ;
}

template <typename ValueType>
void ft_trie<ValueType>::set_success_preserve_errno(int entry_errno) const
{
    (void)entry_errno;
    this->_last_error = FT_ERR_SUCCESSS;
    this->_error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

template <typename ValueType>
int ft_trie<ValueType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());
    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
        {
            if (child_iterator->second->enable_thread_safety() != 0)
            {
                int child_error;

                child_error = child_iterator->second->get_error();
                this->teardown_thread_safety();
                this->set_error(child_error);
                return (-1);
            }
        }
        ++child_iterator;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ValueType>
void ft_trie<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
bool ft_trie<ValueType>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

template <typename ValueType>
int ft_trie<ValueType>::lock(bool *lock_acquired) const
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_trie<ValueType> *>(this)->set_error(ft_errno);
    else
    {
        this->set_success_preserve_errno(ft_errno);
    }
    return (result);
}

template <typename ValueType>
void ft_trie<ValueType>::unlock(bool lock_acquired) const
{
    int mutex_error;

    ft_errno = FT_ERR_SUCCESSS;
    this->unlock_internal(lock_acquired);
    mutex_error = FT_ERR_SUCCESSS;
    if (this->_state_mutex != ft_nullptr)
        mutex_error = this->_state_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        const_cast<ft_trie<ValueType> *>(this)->set_error(mutex_error);
    else
    {
        this->set_success_preserve_errno(ft_errno);
    }
    return ;
}

template <typename ValueType>
int ft_trie<ValueType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template <typename ValueType>
void ft_trie<ValueType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

template <typename ValueType>
void ft_trie<ValueType>::teardown_thread_safety()
{
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
            child_iterator->second->disable_thread_safety();
        ++child_iterator;
    }
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
