#ifndef FT_VECTOR_HPP
#define FT_VECTOR_HPP

#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "constructor.hpp"
#include <cstddef>
#include <utility>
#include "../PThread/mutex.hpp"
#include "../Libft/libft.hpp"

template <typename ElementType>
class ft_vector
{
    private:
        ElementType    *_data;
        size_t        _size;
        size_t        _capacity;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

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
            this->set_error(VECTOR_ALLOC_FAIL);
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
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
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
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
size_t ft_vector<ElementType>::capacity() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t c = this->_capacity;
    this->_mutex.unlock(THREAD_ID);
    return (c);
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_vector<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(const ElementType &value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        size_t newCapacity;
        if (this->_capacity > 0)
            newCapacity = this->_capacity * 2;
        else
            newCapacity = 1;
        this->reserve_internal(newCapacity);
        if (this->_error_code != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], value);
    this->_size++;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::push_back(ElementType &&value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size >= this->_capacity)
    {
        size_t newCapacity;
        if (this->_capacity > 0)
            newCapacity = this->_capacity * 2;
        else
            newCapacity = 1;
        this->reserve_internal(newCapacity);
        if (this->_error_code != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
    }
    construct_at(&this->_data[this->_size], std::forward<ElementType>(value));
    this->_size++;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::pop_back()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size > 0)
    {
        destroy_at(&this->_data[this->_size - 1]);
        --this->_size;
    }
    else
        this->set_error(VECTOR_INVALID_OPERATION);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType& ft_vector<ElementType>::operator[](size_t index)
{
    static ElementType default_instance = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        this->set_error(VECTOR_OUT_OF_BOUNDS);
        this->_mutex.unlock(THREAD_ID);
        return (default_instance);
    }
    ElementType& ref = this->_data[index];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_vector<ElementType>::operator[](size_t index) const
{
    static ElementType default_instance = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (index >= this->_size)
    {
        const_cast<ft_vector<ElementType>*>(this)->set_error(VECTOR_OUT_OF_BOUNDS);
        this->_mutex.unlock(THREAD_ID);
        return (default_instance);
    }
    const ElementType& ref = this->_data[index];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
void ft_vector<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    this->destroy_elements(0, this->_size);
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve_internal(size_t new_capacity)
{
    if (new_capacity > this->_capacity)
    {
        ElementType* new_data = static_cast<ElementType*>(cma_realloc(this->_data,
                    new_capacity * sizeof(ElementType)));
        if (new_data == ft_nullptr)
        {
            this->set_error(VECTOR_ALLOC_FAIL);
            return ;
        }
        this->_data = new_data;
        this->_capacity = new_capacity;
    }
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::reserve(size_t new_capacity)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    this->reserve_internal(new_capacity);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_vector<ElementType>::resize(size_t new_size, const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (new_size < this->_size)
        this->destroy_elements(new_size, this->_size);
    else if (new_size > this->_size)
    {
        this->reserve_internal(new_size);
        if (this->_error_code != ER_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
        size_t index = this->_size;
        while (index < new_size)
        {
            construct_at(&this->_data[index], value);
            ++index;
        }
    }
    this->_size = new_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::insert(iterator pos, const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ft_nullptr);
    }
    size_t index = pos - this->_data;
    if (index > this->_size)
    {
        iterator endIt = this->_data + this->_size;
        this->_mutex.unlock(THREAD_ID);
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
        if (this->_error_code != ER_SUCCESS)
        {
            iterator endIt = this->_data + this->_size;
            this->_mutex.unlock(THREAD_ID);
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
    this->_mutex.unlock(THREAD_ID);
    return (ret);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::erase(iterator pos)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ft_nullptr);
    }
    size_t index = pos - this->_data;
    if (index >= this->_size)
    {
        this->set_error(VECTOR_INVALID_PTR);
        iterator endIt = this->_data + this->_size;
        this->_mutex.unlock(THREAD_ID);
        return (endIt);
    }
    destroy_at(&this->_data[index]);
    size_t i = index;
    while (i < this->_size - 1)
    {
        construct_at(&this->_data[i], this->_data[i + 1]);
        destroy_at(&this->_data[i + 1]);
        ++i;
    }
    --this->_size;
    iterator ret;
    if (index == this->_size)
        ret = this->_data + this->_size;
    else
        ret = &this->_data[index];
    this->_mutex.unlock(THREAD_ID);
    return (ret);
}

template <typename ElementType>
ElementType ft_vector<ElementType>::release_at(size_t index)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (index >= this->_size)
    {
        this->set_error(VECTOR_INVALID_PTR);
        this->_mutex.unlock(THREAD_ID);
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
    this->_mutex.unlock(THREAD_ID);
    return (detached);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::begin()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_data);
    iterator it = this->_data;
    this->_mutex.unlock(THREAD_ID);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::begin() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_data);
    const_iterator it = this->_data;
    this->_mutex.unlock(THREAD_ID);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::iterator ft_vector<ElementType>::end()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_data);
    iterator it = this->_data + this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (it);
}

template <typename ElementType>
typename ft_vector<ElementType>::const_iterator ft_vector<ElementType>::end() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_data);
    const_iterator it = this->_data + this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (it);
}

#endif
