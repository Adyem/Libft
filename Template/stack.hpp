#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstddef>
#include <utility>

template <typename ElementType>
class ft_stack
{
	private:
	    struct StackNode
	    {
	        ElementType data;
	        StackNode* next;
	    };
	
	    StackNode*  _top;
	    size_t      _size;
	    mutable int _errorCode;

	    void    setError(int error) const;

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
    : _top(ft_nullptr), _size(0), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::~ft_stack()
{
    clear();
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack(ft_stack&& other) noexcept
    : _top(other._top), _size(other._size), _errorCode(other._errorCode)
{
    other._top = ft_nullptr;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_stack<ElementType>& ft_stack<ElementType>::operator=(ft_stack&& other) noexcept
{
    if (this != &other)
    {
        clear();
        _top = other._top;
        _size = other._size;
        _errorCode = other._errorCode;
        other._top = ft_nullptr;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
    }
    return (*this);
}

template <typename ElementType>
void ft_stack<ElementType>::setError(int error) const
{
    _errorCode = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    StackNode* newNode = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (newNode == ft_nullptr)
    {
        setError(STACK_ALLOC_FAIL);
        return ;
    }
    construct_at(&newNode->data, value);
    newNode->next = _top;
    _top = newNode;
    ++_size;
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    StackNode* newNode = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (newNode == ft_nullptr)
    {
        setError(STACK_ALLOC_FAIL);
        return ;
    }
    construct_at(&newNode->data, std::move(value));
    newNode->next = _top;
    _top = newNode;
    ++_size;
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    if (_top == ft_nullptr)
    {
        setError(STACK_EMPTY);
        return (ElementType());
    }
    StackNode* node = _top;
    _top = node->next;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
    --_size;
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType errorElement = ElementType();
    if (_top == ft_nullptr)
    {
        setError(STACK_EMPTY);
        return (errorElement);
    }
    return (_top->data);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType errorElement = ElementType();
    if (_top == ft_nullptr)
    {
        setError(STACK_EMPTY);
        return (errorElement);
    }
    return (_top->data);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    return (_size);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    return (_size == 0);
}

template <typename ElementType>
int ft_stack<ElementType>::get_error() const
{
    return (_errorCode);
}

template <typename ElementType>
const char* ft_stack<ElementType>::get_error_str() const
{
    return (ft_strerror(_errorCode));
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    while (_top != ft_nullptr)
    {
        StackNode* node = _top;
        _top = _top->next;
        destroy_at(&node->data);
        cma_free(node);
    }
    _size = 0;
    return ;
}

#endif
