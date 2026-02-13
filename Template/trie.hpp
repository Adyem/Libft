#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../Basic/basic.hpp"
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
        mutable pt_recursive_mutex*                      _mutex;

        int insert_helper(const char *key, int unset_value, ValueType *value_pointer);
        void disable_children_thread_safety();
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
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
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template <typename ValueType>
ft_trie<ValueType>::ft_trie()
    : _data(ft_nullptr), _children(), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
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
}

template <typename ValueType>
int ft_trie<ValueType>::insert_helper(const char *key, int unset_value, ValueType *value_pointer)
{
    if (key == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    size_t key_length = ft_strlen_size_t(key);
    int length_error = ft_global_error_stack_drop_last_error();

    if (length_error != FT_ERR_SUCCESS)
        return (length_error);
    ft_trie<ValueType> *current_node = this;
    const char *key_iterator = key;

    while (*key_iterator)
    {
        char character = *key_iterator;

        if (current_node->_children[character] == ft_nullptr)
        {
            ft_trie<ValueType> *new_child = new (std::nothrow) ft_trie<ValueType>();

            if (new_child == ft_nullptr)
            {
                ft_global_error_stack_push(FT_ERR_NO_MEMORY);
                return (FT_ERR_NO_MEMORY);
            }
            current_node->_children[character] = new_child;
            if (this->_mutex != ft_nullptr)
            {
                int child_result = new_child->enable_thread_safety();

                if (child_result != FT_ERR_SUCCESS)
                {
                    current_node->_children[character] = ft_nullptr;
                    delete new_child;
                    return (child_result);
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
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return (FT_ERR_NO_MEMORY);
        }
    }
    ft_bzero(current_node->_data, sizeof(node_value));
    current_node->_data->_unset_value = unset_value;
    current_node->_data->_key_length = key_length;
    current_node->_data->_value_pointer = value_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int ft_trie<ValueType>::insert(const char *key, ValueType *value_pointer, int unset_value)
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
    int result = this->insert_helper(key, unset_value, value_pointer);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ValueType>
const typename ft_trie<ValueType>::node_value *ft_trie<ValueType>::search(const char *key) const
{
    if (key == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (ft_nullptr);
    }
    const ft_trie<ValueType> *current_node = this;
    const char *key_iterator = key;
    bool key_found = true;

    while (*key_iterator && key_found)
    {
        typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_iterator = current_node->_children.find(*key_iterator);

        if (child_iterator == current_node->_children.end() || child_iterator->second == ft_nullptr)
        {
            key_found = false;
            break;
        }
        current_node = child_iterator->second;
        key_iterator = key_iterator + 1;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (key_found ? current_node->_data : ft_nullptr);
}

template <typename ValueType>
int ft_trie<ValueType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int result = this->prepare_thread_safety();

    if (result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(result);
        return (result);
    }
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
        {
            int child_result = child_iterator->second->enable_thread_safety();

            if (child_result != FT_ERR_SUCCESS)
            {
                this->teardown_thread_safety();
                return (child_result);
            }
        }
        ++child_iterator;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
void ft_trie<ValueType>::disable_children_thread_safety()
{
    typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

    while (child_iterator != this->_children.end())
    {
        if (child_iterator->second != ft_nullptr)
            child_iterator->second->disable_thread_safety();
        ++child_iterator;
    }
}

template <typename ValueType>
void ft_trie<ValueType>::disable_thread_safety()
{
    this->disable_children_thread_safety();
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename ValueType>
bool ft_trie<ValueType>::is_thread_safe_enabled() const
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ValueType>
int ft_trie<ValueType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

template <typename ValueType>
void ft_trie<ValueType>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(result);
}

template <typename ValueType>
int ft_trie<ValueType>::lock_internal(bool *lock_acquired) const
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

template <typename ValueType>
int ft_trie<ValueType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ValueType>
int ft_trie<ValueType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);

    if (result != FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template <typename ValueType>
void ft_trie<ValueType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD

template <typename ValueType>
pt_recursive_mutex *ft_trie<ValueType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
