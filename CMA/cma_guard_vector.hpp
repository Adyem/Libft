#ifndef CMA_GUARD_VECTOR_HPP
#define CMA_GUARD_VECTOR_HPP

#include <cstddef>
#include <cstdlib>
#include <utility>
#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Template/constructor.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"

template <typename t_element>
class cma_guard_vector
{
    private:
        t_element *_data;
        size_t _size;
        size_t _capacity;
        mutable int _error_code;

        void destroy_elements(size_t from, size_t to);
        bool reserve_internal(size_t new_capacity);
        void set_error(int error_code) const;

    public:
        cma_guard_vector(size_t initial_capacity = 0);
        ~cma_guard_vector();

        cma_guard_vector(const cma_guard_vector &other);
        cma_guard_vector &operator=(const cma_guard_vector &other);

        cma_guard_vector(cma_guard_vector &&other) noexcept;
        cma_guard_vector &operator=(cma_guard_vector &&other) noexcept;

        size_t size() const;
        size_t capacity() const;
        bool empty() const;
        int get_error() const;
        const char *get_error_str() const;

        void push_back(const t_element &value);
        void push_back(t_element &&value);
        void pop_back();

        t_element &operator[](size_t index);
        const t_element &operator[](size_t index) const;

        void clear();
        bool reserve(size_t new_capacity);
};

template <typename t_element>
cma_guard_vector<t_element>::cma_guard_vector(size_t initial_capacity)
    : _data(ft_nullptr), _size(0), _capacity(0), _error_code(ER_SUCCESS)
{
    if (initial_capacity > 0)
    {
        this->_data = static_cast<t_element *>(std::malloc(initial_capacity * sizeof(t_element)));
        if (this->_data == ft_nullptr)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->_capacity = initial_capacity;
    }
    return ;
}

template <typename t_element>
cma_guard_vector<t_element>::~cma_guard_vector()
{
    this->destroy_elements(0, this->_size);
    if (this->_data != ft_nullptr)
        std::free(this->_data);
    return ;
}

template <typename t_element>
cma_guard_vector<t_element>::cma_guard_vector(const cma_guard_vector<t_element> &other)
    : _data(ft_nullptr), _size(0), _capacity(0), _error_code(ER_SUCCESS)
{
    if (other._size > 0)
    {
        if (!this->reserve_internal(other._size))
            return ;
        size_t index = 0;
        while (index < other._size)
        {
            try
            {
                construct_at(&this->_data[index], other._data[index]);
            }
            catch (...)
            {
                size_t destroy_index = 0;
                while (destroy_index < index)
                {
                    destroy_at(&this->_data[destroy_index]);
                    destroy_index += 1;
                }
                std::free(this->_data);
                this->_data = ft_nullptr;
                this->_capacity = 0;
                this->set_error(FT_ERR_INTERNAL);
                return ;
            }
            index += 1;
        }
        this->_size = other._size;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename t_element>
cma_guard_vector<t_element> &cma_guard_vector<t_element>::operator=(const cma_guard_vector<t_element> &other)
{
    if (this == &other)
        return (*this);
    cma_guard_vector<t_element> temp(other);
    if (temp.get_error() != ER_SUCCESS && other._size > 0)
    {
        this->set_error(temp.get_error());
        return (*this);
    }
    this->destroy_elements(0, this->_size);
    if (this->_data != ft_nullptr)
        std::free(this->_data);
    this->_data = temp._data;
    this->_size = temp._size;
    this->_capacity = temp._capacity;
    this->_error_code = temp._error_code;
    temp._data = ft_nullptr;
    temp._size = 0;
    temp._capacity = 0;
    temp._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename t_element>
cma_guard_vector<t_element>::cma_guard_vector(cma_guard_vector<t_element> &&other) noexcept
    : _data(other._data), _size(other._size), _capacity(other._capacity), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename t_element>
cma_guard_vector<t_element> &cma_guard_vector<t_element>::operator=(cma_guard_vector<t_element> &&other) noexcept
{
    if (this != &other)
    {
        this->destroy_elements(0, this->_size);
        if (this->_data != ft_nullptr)
            std::free(this->_data);
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

template <typename t_element>
size_t cma_guard_vector<t_element>::size() const
{
    const_cast<cma_guard_vector<t_element> *>(this)->set_error(ER_SUCCESS);
    return (this->_size);
}

template <typename t_element>
size_t cma_guard_vector<t_element>::capacity() const
{
    const_cast<cma_guard_vector<t_element> *>(this)->set_error(ER_SUCCESS);
    return (this->_capacity);
}

template <typename t_element>
bool cma_guard_vector<t_element>::empty() const
{
    const_cast<cma_guard_vector<t_element> *>(this)->set_error(ER_SUCCESS);
    return (this->_size == 0);
}

template <typename t_element>
int cma_guard_vector<t_element>::get_error() const
{
    return (this->_error_code);
}

template <typename t_element>
const char *cma_guard_vector<t_element>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename t_element>
void cma_guard_vector<t_element>::push_back(const t_element &value)
{
    if (this->_size >= this->_capacity)
    {
        size_t new_capacity;
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        if (!this->reserve_internal(new_capacity))
            return ;
    }
    try
    {
        construct_at(&this->_data[this->_size], value);
    }
    catch (...)
    {
        this->set_error(FT_ERR_INTERNAL);
        return ;
    }
    this->_size += 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename t_element>
void cma_guard_vector<t_element>::push_back(t_element &&value)
{
    if (this->_size >= this->_capacity)
    {
        size_t new_capacity;
        if (this->_capacity > 0)
            new_capacity = this->_capacity * 2;
        else
            new_capacity = 1;
        if (!this->reserve_internal(new_capacity))
            return ;
    }
    try
    {
        construct_at(&this->_data[this->_size], std::forward<t_element>(value));
    }
    catch (...)
    {
        this->set_error(FT_ERR_INTERNAL);
        return ;
    }
    this->_size += 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename t_element>
void cma_guard_vector<t_element>::pop_back()
{
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return ;
    }
    destroy_at(&this->_data[this->_size - 1]);
    this->_size -= 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename t_element>
t_element &cma_guard_vector<t_element>::operator[](size_t index)
{
    static t_element default_instance = t_element();
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    this->set_error(ER_SUCCESS);
    return (this->_data[index]);
}

template <typename t_element>
const t_element &cma_guard_vector<t_element>::operator[](size_t index) const
{
    static t_element default_instance = t_element();
    if (index >= this->_size)
    {
        const_cast<cma_guard_vector<t_element> *>(this)->set_error(FT_ERR_OUT_OF_RANGE);
        return (default_instance);
    }
    this->set_error(ER_SUCCESS);
    return (this->_data[index]);
}

template <typename t_element>
void cma_guard_vector<t_element>::clear()
{
    this->destroy_elements(0, this->_size);
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename t_element>
bool cma_guard_vector<t_element>::reserve(size_t new_capacity)
{
    if (!this->reserve_internal(new_capacity))
        return (false);
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename t_element>
void cma_guard_vector<t_element>::destroy_elements(size_t from, size_t to)
{
    size_t index = from;
    while (index < to)
    {
        destroy_at(&this->_data[index]);
        index += 1;
    }
    return ;
}

template <typename t_element>
bool cma_guard_vector<t_element>::reserve_internal(size_t new_capacity)
{
    if (new_capacity <= this->_capacity)
        return (true);
    t_element *new_data = static_cast<t_element *>(std::malloc(new_capacity * sizeof(t_element)));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t index = 0;
    while (index < this->_size)
    {
        try
        {
            construct_at(&new_data[index], ft_move(this->_data[index]));
        }
        catch (...)
        {
            size_t destroy_index = 0;
            while (destroy_index < index)
            {
                destroy_at(&new_data[destroy_index]);
                destroy_index += 1;
            }
            std::free(new_data);
            this->set_error(FT_ERR_INTERNAL);
            return (false);
        }
        index += 1;
    }
    size_t destroy_index = 0;
    while (destroy_index < this->_size)
    {
        destroy_at(&this->_data[destroy_index]);
        destroy_index += 1;
    }
    if (this->_data != ft_nullptr)
        std::free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return (true);
}

template <typename t_element>
void cma_guard_vector<t_element>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

#endif
