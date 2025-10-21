#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "swap.hpp"
#include <cstddef>
#include <utility>
#include <functional>
#include "move.hpp"

template <typename ElementType, typename Compare = std::less<ElementType> >
class ft_priority_queue
{
    private:
        ElementType*   _data;
        size_t         _capacity;
        size_t         _size;
        Compare        _comp;
        mutable int    _error_code;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired);
        void    heapify_up(size_t index);
        void    heapify_down(size_t index);

    public:
        ft_priority_queue(size_t initialCapacity = 0, const Compare& comp = Compare());
        ~ft_priority_queue();

        ft_priority_queue(const ft_priority_queue&) = delete;
        ft_priority_queue& operator=(const ft_priority_queue&) = delete;

        ft_priority_queue(ft_priority_queue&& other) noexcept;
        ft_priority_queue& operator=(ft_priority_queue&& other) noexcept;

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

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(size_t initialCapacity, const Compare& comp)
    : _data(ft_nullptr), _capacity(0), _size(0), _comp(comp), _error_code(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initialCapacity));
        if (this->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
            return ;
        }
        this->_capacity = initialCapacity;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::~ft_priority_queue()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(ft_priority_queue&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size), _comp(other._comp), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>& ft_priority_queue<ElementType, Compare>::operator=(ft_priority_queue&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_comp = other._comp;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
    {
        this->set_error(ER_SUCCESS);
        return (true);
    }
    size_t new_capacity;
    if (this->_capacity == 0)
        new_capacity = 1;
    else
        new_capacity = this->_capacity * 2;
    while (new_capacity < desired)
        new_capacity *= 2;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
        return (false);
    }
    size_t element_index = 0;
    while (element_index < this->_size)
    {
        construct_at(&new_data[element_index], ft_move(this->_data[element_index]));
        destroy_at(&this->_data[element_index]);
        element_index += 1;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_up(size_t index)
{
    while (index > 0)
    {
        size_t parent_index = (index - 1) / 2;
        if (!this->_comp(this->_data[parent_index], this->_data[index]))
            break;
        ft_swap(this->_data[parent_index], this->_data[index]);
        index = parent_index;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_down(size_t index)
{
    while (true)
    {
        size_t left_child_index = index * 2 + 1;
        size_t right_child_index = left_child_index + 1;
        size_t largest_index = index;
        if (left_child_index < this->_size && this->_comp(this->_data[largest_index], this->_data[left_child_index]))
            largest_index = left_child_index;
        if (right_child_index < this->_size && this->_comp(this->_data[largest_index], this->_data[right_child_index]))
            largest_index = right_child_index;
        if (largest_index == index)
            break;
        ft_swap(this->_data[index], this->_data[largest_index]);
        index = largest_index;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(const ElementType& value)
{
    if (!this->ensure_capacity(this->_size + 1))
        return ;
    construct_at(&this->_data[this->_size], value);
    this->heapify_up(this->_size);
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(ElementType&& value)
{
    if (!this->ensure_capacity(this->_size + 1))
        return ;
    construct_at(&this->_data[this->_size], ft_move(value));
    this->heapify_up(this->_size);
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType, typename Compare>
ElementType ft_priority_queue<ElementType, Compare>::pop()
{
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        return (ElementType());
    }
    ElementType top_value = ft_move(this->_data[0]);
    destroy_at(&this->_data[0]);
    --this->_size;
    if (this->_size > 0)
    {
        construct_at(&this->_data[0], ft_move(this->_data[this->_size]));
        destroy_at(&this->_data[this->_size]);
        this->heapify_down(0);
    }
    this->set_error(ER_SUCCESS);
    return (top_value);
}

template <typename ElementType, typename Compare>
ElementType& ft_priority_queue<ElementType, Compare>::top()
{
    static ElementType error_element = ElementType();
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        return (error_element);
    }
    ElementType& value = this->_data[0];
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType, typename Compare>
const ElementType& ft_priority_queue<ElementType, Compare>::top() const
{
    static ElementType error_element = ElementType();
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        return (error_element);
    }
    const ElementType& value = this->_data[0];
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType, typename Compare>
size_t ft_priority_queue<ElementType, Compare>::size() const
{
    size_t current_size = this->_size;
    this->set_error(ER_SUCCESS);
    return (current_size);
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::empty() const
{
    bool result = (this->_size == 0);
    this->set_error(ER_SUCCESS);
    return (result);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

template <typename ElementType, typename Compare>
const char* ft_priority_queue<ElementType, Compare>::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::clear()
{
    size_t element_index = 0;
    while (element_index < this->_size)
    {
        destroy_at(&this->_data[element_index]);
        element_index += 1;
    }
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
