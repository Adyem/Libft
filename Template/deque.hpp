#ifndef FT_DEQUE_HPP
#define FT_DEQUE_HPP

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
** - push_front, push_back: O(1); only invalidates references to previous front/back respectively.
** - pop_front, pop_back: O(1); invalidates references to removed element.
** - clear: O(n); invalidates all node references.
*/
template <typename ElementType>
class ft_deque
{
    private:
        struct DequeNode
        {
            ElementType _data;
            DequeNode* _prev;
            DequeNode* _next;
        };

        DequeNode*   _front;
        DequeNode*   _back;
        size_t       _size;
        mutable int  _error_code;

        void    set_error(int error) const;

    public:
        ft_deque();
        ~ft_deque();

        ft_deque(const ft_deque&) = delete;
        ft_deque& operator=(const ft_deque&) = delete;

        ft_deque(ft_deque&& other) noexcept;
        ft_deque& operator=(ft_deque&& other) noexcept;

        void push_front(const ElementType& value);
        void push_front(ElementType&& value);
        void push_back(const ElementType& value);
        void push_back(ElementType&& value);
        ElementType pop_front();
        ElementType pop_back();

        ElementType& front();
        const ElementType& front() const;
        ElementType& back();
        const ElementType& back() const;

        size_t size() const;
        bool empty() const;

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType>
ft_deque<ElementType>::ft_deque()
    : _front(ft_nullptr), _back(ft_nullptr), _size(0), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::~ft_deque()
{
    this->clear();
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::ft_deque(ft_deque&& other) noexcept
    : _front(other._front), _back(other._back), _size(other._size), _error_code(other._error_code)
{
    other._front = ft_nullptr;
    other._back = ft_nullptr;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_deque<ElementType>& ft_deque<ElementType>::operator=(ft_deque&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        this->_front = other._front;
        this->_back = other._back;
        this->_size = other._size;
        this->_error_code = other._error_code;
        other._front = ft_nullptr;
        other._back = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        this->set_error(ER_SUCCESS);
    }
    return (*this);
}

template <typename ElementType>
void ft_deque<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(const ElementType& value)
{
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, value);
    node->_prev = ft_nullptr;
    node->_next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->_prev = node;
    this->_front = node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(ElementType&& value)
{
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, std::move(value));
    node->_prev = ft_nullptr;
    node->_next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->_prev = node;
    this->_front = node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(const ElementType& value)
{
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, value);
    node->_next = ft_nullptr;
    node->_prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->_next = node;
    this->_back = node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(ElementType&& value)
{
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, std::move(value));
    node->_next = ft_nullptr;
    node->_prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->_next = node;
    this->_back = node;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_front()
{
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ElementType());
    }
    DequeNode* node = this->_front;
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_back = ft_nullptr;
    else
        this->_front->_prev = ft_nullptr;
    ElementType value = std::move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_back()
{
    if (this->_back == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ElementType());
    }
    DequeNode* node = this->_back;
    this->_back = node->_prev;
    if (this->_back == ft_nullptr)
        this->_front = ft_nullptr;
    else
        this->_back->_next = ft_nullptr;
    ElementType value = std::move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::front()
{
    static ElementType error_element = ElementType();
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    ElementType& reference = this->_front->_data;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    const ElementType& reference = this->_front->_data;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::back()
{
    static ElementType error_element = ElementType();
    if (this->_back == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    ElementType& reference = this->_back->_data;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::back() const
{
    static ElementType error_element = ElementType();
    if (this->_back == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    const ElementType& reference = this->_back->_data;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ElementType>
size_t ft_deque<ElementType>::size() const
{
    size_t current_size = this->_size;
    const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (current_size);
}

template <typename ElementType>
bool ft_deque<ElementType>::empty() const
{
    bool result = (this->_size == 0);
    const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename ElementType>
int ft_deque<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_deque<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_deque<ElementType>::clear()
{
    while (this->_front != ft_nullptr)
    {
        DequeNode* node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_back = ft_nullptr;
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
