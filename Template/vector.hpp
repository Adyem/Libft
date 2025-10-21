#ifndef FT_VECTOR_HPP
#define FT_VECTOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <utility>
#include "../Libft/libft.hpp"
#include "move.hpp"

template <typename ElementType>
class ft_vector
{
    private:
        ElementType    *_data;
        size_t        _size;
        size_t        _capacity;
        mutable int     _error_code;

        void    destroy_elements(size_t from, size_t to);
        void    reserve_internal(size_t new_capacity);

    protected:
        void    set_error(int error_code) const;
        ElementType release_at(size_t index);

    public:
        using iterator = ElementType*;
        using const_iterator = const ElementType*;

        ft_vector(size_t initial_capacity = 0);
        ~ft_vector();

        ft_vector(const ft_vector&) = delete;
        ft_vector& operator=(const ft_vector&) = delete;

        ft_vector(ft_vector&& other) noexcept;
        ft_vector& operator=(ft_vector&& other) noexcept;

        size_t size() const;
        size_t capacity() const;
        bool empty() const;
        int get_error() const;
        const char* get_error_str() const;

        void push_back(const ElementType &value);
        void push_back(ElementType &&value);
        void pop_back();

        ElementType& operator[](size_t index);
        const ElementType& operator[](size_t index) const;

        void clear();
        void reserve(size_t new_capacity);
        void resize(size_t new_size, const ElementType& value = ElementType());

        iterator insert(iterator pos, const ElementType& value);
        iterator erase(iterator pos);
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
};

template <typename ElementType>
ft_vector<ElementType>::ft_vector(size_t initial_capacity)
    : _data(ft_nullptr), _size(0), _capacity(0), _error_code(ER_SUCCESS)
{
    if (initial_capacity > 0)
    {
        this->_data = static_cast<ElementType*>
            (cma_malloc(initial_capacity * sizeof(ElementType)));
        if (this->_data == ft_nullptr)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->_capacity = initial_capacity;
    }
    return ;
}

template <typename ElementType>
ft_vector<ElementType>::~ft_vector()
{
    this->destroy_elements(0, this->_size);
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

template <typename ElementType>
ft_vector<ElementType>::ft_vector(ft_vector<ElementType>&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity),
      _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._error_code = ER_SUCCESS;
}

template <typename ElementType>
ft_vector<ElementType>& ft_vector<ElementType>::operator=(ft_vector<ElementType>&& other) noexcept
{
    if (this != &other)
    {
        this->destroy_elements(0, this->_size);
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->_data = other._data;
        this->_size = other._size;
        this->_capacity = other._capacity;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._size = 0;
        other._capacity = 0;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename ElementType>
void ft_vector<ElementType>::destroy_elements(size_t from, size_t to)
{
    size_t index = from;
    while (index < to)
    {
        destroy_at(&this->_data[index]);
        ++index;
    }
    return ;
}

template <typename ElementType>
size_t ft_vector<ElementType>::size() const
{
    const_cast<ft_vector<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (this->_size);
}

template <typename ElementType>
size_t ft_vector<ElementType>::capacity() const
{
    const_cast<ft_vector<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (this->_capacity);
}

template <typename ElementType>
bool ft_vector<ElementType>::empty() const
{
    const_cast<ft_vector<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (this->_size == 0);
}

template <typename ElementType>
void ft_vector<ElementType>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename ElementType>
int ft_vector<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_vector<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(const ElementType &value)
{
    if (this->_size >= this->_capacity)
    {
        size_t newCapacity;
        if (this->_capacity > 0)
            newCapacity = this->_capacity * 2;
        else
            newCapacity = 1;
        this->reserve_internal(newCapacity);
        if (this->_capacity < newCapacity)
            return ;
    }
    construct_at(&this->_data[this->_size], value);
    this->_size++;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(ElementType &&value)
{
    if (this->_size >= this->_capacity)
    {
        size_t newCapacity;
        if (this->_capacity > 0)
            newCapacity = this->_capacity * 2;
        else
            newCapacity = 1;
        this->reserve_internal(newCapacity);
        if (this->_capacity < newCapacity)
            return ;
    }
    construct_at(&this->_data[this->_size], std::forward<ElementType>(value));
    this->_size++;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::pop_back()
{
    if (this->_size > 0)
    {
        destroy_at(&this->_data[this->_size - 1]);
        --this->_size;
        this->set_error(ER_SUCCESS);
    }
    else
        this->set_error(FT_ERR_INVALID_OPERATION);
    return ;
}

template <typename ElementType>
ElementType& ft_vector<ElementType>::operator[](size_t index)
{
    static ElementType default_instance = ElementType();
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    ElementType& ref = this->_data[index];
    this->set_error(ER_SUCCESS);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_vector<ElementType>::operator[](size_t index) const
{
    static ElementType default_instance = ElementType();
    if (index >= this->_size)
    {
        const_cast<ft_vector<ElementType>*>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    const ElementType& ref = this->_data[index];
    this->set_error(ER_SUCCESS);
    return (ref);
}

template <typename ElementType>
void ft_vector<ElementType>::clear()
{
    this->destroy_elements(0, this->_size);
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve_internal(size_t new_capacity)
{
    if (new_capacity <= this->_capacity)
        return ;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(new_capacity * sizeof(ElementType)));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    ElementType* old_data = this->_data;
    size_t current_size = this->_size;
    size_t index = 0;
    try
    {
        while (index < current_size)
        {
            construct_at(&new_data[index], ft_move(old_data[index]));
            ++index;
        }
    }
    catch (...)
    {
        while (index > 0)
        {
            --index;
            destroy_at(&new_data[index]);
        }
        cma_free(new_data);
        throw;
    }
    size_t destroy_index = 0;
    while (destroy_index < current_size)
    {
        destroy_at(&old_data[destroy_index]);
        ++destroy_index;
    }
    if (old_data != ft_nullptr)
        cma_free(old_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve(size_t new_capacity)
{
    this->reserve_internal(new_capacity);
    if (this->_capacity >= new_capacity)
        this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::resize(size_t new_size, const ElementType& value)
{
    if (new_size < this->_size)
        this->destroy_elements(new_size, this->_size);
    else if (new_size > this->_size)
    {
        this->reserve_internal(new_size);
        if (this->_capacity < new_size)
            return ;
        size_t index = this->_size;
        while (index < new_size)
        {
            construct_at(&this->_data[index], value);
            ++index;
        }
    }
    this->_size = new_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::insert(iterator pos, const ElementType& value)
{
    size_t index = pos - this->_data;
    if (index > this->_size)
    {
        iterator endIt = this->_data + this->_size;
        return (endIt);
    }
    if (this->_size >= this->_capacity)
    {
        size_t new_capacity;
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        this->reserve_internal(new_capacity);
        if (this->_capacity < new_capacity)
        {
            iterator endIt = this->_data + this->_size;
            return (endIt);
        }
        pos = this->_data + index;
    }
    size_t i = this->_size;
    while (i > index)
    {
        construct_at(&this->_data[i], this->_data[i - 1]);
        destroy_at(&this->_data[i - 1]);
        i--;
    }
    construct_at(&this->_data[index], value);
    this->_size++;
    iterator ret = &this->_data[index];
    this->set_error(ER_SUCCESS);
    return (ret);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::erase(iterator pos)
{
    size_t index = pos - this->_data;
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        iterator endIt = this->_data + this->_size;
        return (endIt);
    }
    destroy_at(&this->_data[index]);
    size_t shift_index = index;
    while (shift_index < this->_size - 1)
    {
        construct_at(&this->_data[shift_index], ft_move(this->_data[shift_index + 1]));
        destroy_at(&this->_data[shift_index + 1]);
        ++shift_index;
    }
    --this->_size;
    iterator ret;
    if (index >= this->_size)
        ret = this->_data + this->_size;
    else
        ret = &this->_data[index];
    this->set_error(ER_SUCCESS);
    return (ret);
}

template <typename ElementType>
ElementType ft_vector<ElementType>::release_at(size_t index)
{
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        return (ElementType());
    }
    ElementType detached = ft_move(this->_data[index]);
    size_t i2 = index;
    while (i2 < this->_size - 1)
    {
        this->_data[i2] = ft_move(this->_data[i2 + 1]);
        ++i2;
    }
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (detached);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::begin()
{
    iterator it = this->_data;
    this->set_error(ER_SUCCESS);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::begin() const
{
    const_iterator it = this->_data;
    this->set_error(ER_SUCCESS);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::end()
{
    iterator it = this->_data + this->_size;
    this->set_error(ER_SUCCESS);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::end() const
{
    const_iterator it = this->_data + this->_size;
    this->set_error(ER_SUCCESS);
    return (it);
}

#endif
