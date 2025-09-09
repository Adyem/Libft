#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
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
            ElementType _data;
            StackNode* _next;
        };

            StackNode*  _top;
            size_t      _size;
            mutable int _error_code;
            mutable pt_mutex _mutex;

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
        this->_error_code = other._error_code;
        other._top = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    StackNode* new_node = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->set_error(STACK_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&new_node->_data, value);
    new_node->_next = this->_top;
    this->_top = new_node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    StackNode* new_node = static_cast<StackNode*>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->set_error(STACK_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&new_node->_data, std::move(value));
    new_node->_next = this->_top;
    this->_top = new_node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_top == ft_nullptr)
    {
        this->set_error(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    StackNode* node = this->_top;
    this->_top = node->_next;
    ElementType value = std::move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        this->set_error(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    ElementType& value = this->_top->_data;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        this->set_error(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    const ElementType& value = this->_top->_data;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t current_size = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (current_size);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool result = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
int ft_stack<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_error_code);
    int error_value = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (error_value);
}

template <typename ElementType>
const char* ft_stack<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_error_code));
    int error_value = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(error_value));
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    while (this->_top != ft_nullptr)
    {
        StackNode* node = this->_top;
        this->_top = this->_top->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
