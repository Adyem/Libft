#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
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
            mutable pt_mutex _mutex;

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
    this->clear();
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
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        this->clear();
        this->_top = other._top;
        this->_size = other._size;
        this->_errorCode = other._errorCode;
        other._top = ft_nullptr;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
void ft_stack<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    StackNode* newNode = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (newNode == ft_nullptr)
    {
        this->setError(STACK_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&newNode->data, value);
    newNode->next = this->_top;
    this->_top = newNode;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    StackNode* newNode = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (newNode == ft_nullptr)
    {
        this->setError(STACK_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&newNode->data, std::move(value));
    newNode->next = this->_top;
    this->_top = newNode;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_top == ft_nullptr)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    StackNode* node = this->_top;
    this->_top = node->next;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_top == ft_nullptr)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_top->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_top == ft_nullptr)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_top->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
int ft_stack<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_stack<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    while (this->_top != ft_nullptr)
    {
        StackNode* node = this->_top;
        this->_top = this->_top->next;
        destroy_at(&node->data);
        cma_free(node);
    }
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
