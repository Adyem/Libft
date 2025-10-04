#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "unordened_map.hpp"
#include <cstddef>
#include <new>

int pt_thread_yield();

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
        ft_unord_map<char, ft_trie<ValueType>*>          _children;
        mutable pt_mutex                                 _mutex;
        mutable int                                      _error_code;
        mutable int                                      _last_error;

        int insert_helper(const char *key, int unset_value, ValueType *value_pointer);
        void set_error(int error) const;

    public:
        ft_trie();
        ~ft_trie();

        int insert(const char *key, ValueType *value_pointer, int unset_value = 0);
        const node_value *search(const char *key) const;
        int get_error() const;
        const char *get_error_str() const;
};

template <typename ValueType>
ft_trie<ValueType>::ft_trie()
        : _data(ft_nullptr), _children(), _mutex(), _error_code(ER_SUCCESS), _last_error(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::~ft_trie()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    typename ft_unord_map<char, ft_trie<ValueType>*>::iterator child_iterator = this->_children.begin();
    while (child_iterator != this->_children.end())
    {
        delete child_iterator->second;
        ++child_iterator;
    }
    if (this->_data != ft_nullptr)
    {
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
int ft_trie<ValueType>::insert_helper(const char *key, int unset_value, ValueType *value_pointer)
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (1);
        }
        if (this->_error_code != ER_SUCCESS)
        {
            this->set_error(this->_error_code);
            return (1);
        }
    }
    if (key == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (1);
    }
    size_t key_length = ft_strlen_size_t(key);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        return (1);
    }
    ft_trie<ValueType> *current_node = this;
    const char *key_iterator = key;
    while (*key_iterator)
    {
        ft_unique_lock<pt_mutex> guard(current_node->_mutex);
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (1);
        }
        char character = *key_iterator;
        if (current_node->_children[character] == ft_nullptr)
        {
            ft_trie<ValueType> *new_child = new (std::nothrow) ft_trie<ValueType>();
            if (new_child == ft_nullptr)
            {
                this->set_error(FT_EALLOC);
                return (1);
            }
            current_node->_children[character] = new_child;
        }
        ft_trie<ValueType> *next_node = current_node->_children[character];
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (1);
        }
        current_node = next_node;
        key_iterator++;
    }
    ft_unique_lock<pt_mutex> final_guard(current_node->_mutex);
    if (final_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(final_guard.get_error());
        return (1);
    }
    if (current_node->_data == ft_nullptr)
    {
        current_node->_data = static_cast<node_value *>(cma_malloc(sizeof(node_value)));
        if (current_node->_data == ft_nullptr)
        {
            this->set_error(FT_EALLOC);
            return (1);
        }
    }
    ft_bzero(current_node->_data, sizeof(node_value));
    current_node->_data->_unset_value = unset_value;
    current_node->_data->_key_length = key_length;
    current_node->_data->_value_pointer = value_pointer;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename ValueType>
int ft_trie<ValueType>::insert(const char *key, ValueType *value_pointer, int unset_value)
{
    int result = this->insert_helper(key, unset_value, value_pointer);
    if (result == 0)
        this->set_error(ER_SUCCESS);
    return (result);
}

template <typename ValueType>
const typename ft_trie<ValueType>::node_value *ft_trie<ValueType>::search(const char *key) const
{
    if (key == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    int attempt;
    attempt = 0;
    while (attempt < 512)
    {
        const ft_trie<ValueType> *current_node;
        current_node = this;
        const char *key_iterator;
        key_iterator = key;
        bool missing_child;
        missing_child = false;
        while (*key_iterator && !missing_child)
        {
            ft_unique_lock<pt_mutex> guard(current_node->_mutex);
            if (guard.get_error() != ER_SUCCESS)
            {
                this->set_error(guard.get_error());
                return (ft_nullptr);
            }
            typename ft_unord_map<char, ft_trie<ValueType>*>::const_iterator child_iterator(current_node->_children.find(*key_iterator));
            if (child_iterator == current_node->_children.end())
            {
                this->set_error(ER_SUCCESS);
                missing_child = true;
            }
            else
            {
                const ft_trie<ValueType> *next_node;
                next_node = child_iterator->second;
                if (next_node == ft_nullptr)
                {
                    this->set_error(ER_SUCCESS);
                    missing_child = true;
                }
                else
                {
                    current_node = next_node;
                    key_iterator = key_iterator + 1;
                }
            }
        }
        if (!missing_child && *key_iterator == '\0')
        {
            ft_unique_lock<pt_mutex> final_guard(current_node->_mutex);
            if (final_guard.get_error() != ER_SUCCESS)
            {
                this->set_error(final_guard.get_error());
                return (ft_nullptr);
            }
            if (current_node->_data != ft_nullptr)
            {
                this->set_error(ER_SUCCESS);
                return (current_node->_data);
            }
            this->set_error(ER_SUCCESS);
        }
        pt_thread_yield();
        attempt = attempt + 1;
    }
    this->set_error(ER_SUCCESS);
    return (ft_nullptr);
}

template <typename ValueType>
int ft_trie<ValueType>::get_error() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        return (guard.get_error());
    }
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
    if (error == ER_SUCCESS)
    {
        this->_error_code = ER_SUCCESS;
        return ;
    }
    this->_error_code = 1;
    return ;
}

#endif
