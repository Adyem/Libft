#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cstddef>
#include <utility>

/*
** Complexity and iterator invalidation guarantees:
** - size, empty: O(1) without invalidation.
** - push: O(1); invalidates only the previous top reference.
** - pop: O(1); invalidates references to removed top element.
** - top accessors: O(1); no invalidation.
** - clear: O(n); invalidates all node references.
** Thread safety: no internal synchronization; callers must provide external protection when sharing instances.
*/
template <typename ElementType>
class ft_stack
{
    private:
        struct StackNode
        {
            ElementType _data;
            StackNode* _next;
        };

            StackNode*  _top;
            size_t      _size;
            mutable int _error_code;

        void    set_error(int error) const;

    public:
        ft_stack();
        ~ft_stack();

        ft_stack(const ft_stack&) = delete;
        ft_stack& operator=(const ft_stack&) = delete;

        ft_stack(ft_stack&& other) noexcept;
        ft_stack& operator=(ft_stack&& other) noexcept;

            void push(const ElementType& value);
            void push(ElementType&& value);
            ElementType pop();

            ElementType& top();
            const ElementType& top() const;

            size_t size() const;
            bool empty() const;

            int get_error() const;
            const char* get_error_str() const;

            void clear();
};

template <typename ElementType>
ft_stack<ElementType>::ft_stack()
    : _top(ft_nullptr), _size(0), _error_code(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::~ft_stack()
{
    this->clear();
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack(ft_stack&& other) noexcept
    : _top(other._top), _size(other._size), _error_code(other._error_code)
{
    other._top = ft_nullptr;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_stack<ElementType>& ft_stack<ElementType>::operator=(ft_stack&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        this->_top = other._top;
        this->_size = other._size;
        this->_error_code = other._error_code;
        other._top = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename ElementType>
void ft_stack<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    StackNode* new_node = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&new_node->_data, value);
    new_node->_next = this->_top;
    this->_top = new_node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    StackNode* new_node = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&new_node->_data, std::move(value));
    new_node->_next = this->_top;
    this->_top = new_node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    if (this->_top == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ElementType());
    }
    StackNode* node = this->_top;
    this->_top = node->_next;
    ElementType value = std::move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType error_element = ElementType();
    if (this->_top == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    ElementType& value = this->_top->_data;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType error_element = ElementType();
    if (this->_top == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    const ElementType& value = this->_top->_data;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    size_t current_size;

    current_size = this->_size;
    const_cast<ft_stack<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (current_size);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    bool result;

    result = (this->_size == 0);
    const_cast<ft_stack<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename ElementType>
int ft_stack<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_stack<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    while (this->_top != ft_nullptr)
    {
        StackNode* node = this->_top;
        this->_top = this->_top->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
