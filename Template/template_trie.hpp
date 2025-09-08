#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "template_unordened_map.hpp"
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
        int                                              _error;
        ft_unord_map<char, ft_trie<ValueType>*>          _children;

        int insert_helper(const char *key, int unset_value, ValueType *value_pointer);

    public:
        ft_trie();
        ~ft_trie();

        int insert(const char *key, ValueType *value_pointer, int unset_value = 0);
        const node_value *search(const char *key) const;
        int get_error() const;
};

template <typename ValueType>
ft_trie<ValueType>::ft_trie()
    : _data(ft_nullptr), _error(0), _children()
{
    return ;
}

template <typename ValueType>
ft_trie<ValueType>::~ft_trie()
{
    typename ft_unord_map<char, ft_trie<ValueType>*>::iterator child_iterator = this->_children.begin();
    while (child_iterator != this->_children.end())
    {
        delete child_iterator->second;
        ++child_iterator;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

template <typename ValueType>
int ft_trie<ValueType>::insert_helper(const char *key, int unset_value, ValueType *value_pointer)
{
    if (this->_error)
        return (1);
    size_t key_length = ft_strlen_size_t(key);
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
                this->_error = 1;
                return (1);
            }
            current_node->_children[character] = new_child;
        }
        current_node = current_node->_children[character];
        key_iterator++;
    }
    if (current_node->_data == ft_nullptr)
    {
        current_node->_data = static_cast<node_value *>(cma_malloc(sizeof(node_value)));
        if (current_node->_data == ft_nullptr)
        {
            this->_error = 1;
            return (1);
        }
    }
    ft_bzero(current_node->_data, sizeof(node_value));
    current_node->_data->_unset_value = unset_value;
    current_node->_data->_key_length = key_length;
    current_node->_data->_value_pointer = value_pointer;
    return (0);
}

template <typename ValueType>
int ft_trie<ValueType>::insert(const char *key, ValueType *value_pointer, int unset_value)
{
    return (this->insert_helper(key, unset_value, value_pointer));
}

template <typename ValueType>
const typename ft_trie<ValueType>::node_value *ft_trie<ValueType>::search(const char *key) const
{
    if (this->_error)
        return (ft_nullptr);
    const ft_trie<ValueType> *current_node = this;
    const char *key_iterator = key;
    while (*key_iterator)
    {
        typename ft_unord_map<char, ft_trie<ValueType>*>::const_iterator child_iterator = current_node->_children.find(*key_iterator);
        if (child_iterator == current_node->_children.end())
            return (ft_nullptr);
        current_node = child_iterator->second;
        key_iterator++;
    }
    return (current_node->_data);
}

template <typename ValueType>
int ft_trie<ValueType>::get_error() const
{
    return (this->_error);
}

#endif
