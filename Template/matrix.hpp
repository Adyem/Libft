#ifndef FT_MATRIX_HPP
#define FT_MATRIX_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>

/*
 * ft_matrix
 * Basic matrix operations with thread-safe error reporting.
 */

template <typename ElementType>
class ft_matrix
{
    private:
        ElementType*   _data;
        size_t         _rows;
        size_t         _cols;
        mutable int    _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error) const;

    public:
        ft_matrix(size_t rows = 0, size_t cols = 0);
        ~ft_matrix();

        ft_matrix(const ft_matrix&) = delete;
        ft_matrix& operator=(const ft_matrix&) = delete;

        ft_matrix(ft_matrix&& other) noexcept;
        ft_matrix& operator=(ft_matrix&& other) noexcept;

        bool    init(size_t rows, size_t cols);
        ElementType& at(size_t r, size_t c);
        const ElementType& at(size_t r, size_t c) const;
        size_t  rows() const;
        size_t  cols() const;

        ft_matrix add(const ft_matrix& other) const;
        ft_matrix multiply(const ft_matrix& other) const;
        ft_matrix transpose() const;
        ElementType determinant() const;

        int get_error() const;
        const char* get_error_str() const;
        void clear();
};

// Implementation

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(size_t rows, size_t cols)
    : _data(ft_nullptr), _rows(0), _cols(0), _error_code(ER_SUCCESS)
{
    if (rows > 0 && cols > 0)
        this->init(rows, cols);
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>::~ft_matrix()
{
    this->clear();
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(ft_matrix&& other) noexcept
    : _data(other._data), _rows(other._rows), _cols(other._cols), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._rows = 0;
    other._cols = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>& ft_matrix<ElementType>::operator=(ft_matrix&& other) noexcept
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
        this->_data = other._data;
        this->_rows = other._rows;
        this->_cols = other._cols;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._rows = 0;
        other._cols = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
bool ft_matrix<ElementType>::init(size_t rows, size_t cols)
{
    size_t total = rows * cols;
    this->clear();
    if (total == 0)
    {
        this->_rows = rows;
        this->_cols = cols;
        return (true);
    }
    this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * total));
    if (this->_data == ft_nullptr)
    {
        this->set_error(MATRIX_ALLOC_FAIL);
        return (false);
    }
    size_t i = 0;
    while (i < total)
    {
        construct_default_at(&this->_data[i]);
        ++i;
    }
    this->_rows = rows;
    this->_cols = cols;
    return (true);
}

template <typename ElementType>
ElementType& ft_matrix<ElementType>::at(size_t r, size_t c)
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (error_element);
    if (r >= this->_rows || c >= this->_cols)
    {
        this->set_error(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    ElementType& element = this->_data[r * this->_cols + c];
    this->_mutex.unlock(THREAD_ID);
    return (element);
}

template <typename ElementType>
const ElementType& ft_matrix<ElementType>::at(size_t r, size_t c) const
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (error_element);
    if (r >= this->_rows || c >= this->_cols)
    {
        this->set_error(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    const ElementType& element = this->_data[r * this->_cols + c];
    this->_mutex.unlock(THREAD_ID);
    return (element);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::rows() const
{
    return (this->_rows);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::cols() const
{
    return (this->_cols);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::add(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (result);
    if (other._mutex.lock(THREAD_ID) != SUCCES)
    {
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (this->_rows != other._rows || this->_cols != other._cols)
    {
        this->set_error(MATRIX_DIM_MISMATCH);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (!result.init(this->_rows, this->_cols))
    {
        this->set_error(MATRIX_ALLOC_FAIL);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    size_t total = this->_rows * this->_cols;
    size_t i = 0;
    while (i < total)
    {
        result._data[i] = this->_data[i] + other._data[i];
        ++i;
    }
    other._mutex.unlock(THREAD_ID);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::multiply(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (result);
    if (other._mutex.lock(THREAD_ID) != SUCCES)
    {
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (this->_cols != other._rows)
    {
        this->set_error(MATRIX_DIM_MISMATCH);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (!result.init(this->_rows, other._cols))
    {
        this->set_error(MATRIX_ALLOC_FAIL);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    size_t row_index = 0;
    while (row_index < this->_rows)
    {
        size_t column_index = 0;
        while (column_index < other._cols)
        {
            ElementType sum = ElementType();
            size_t inner_index = 0;
            while (inner_index < this->_cols)
            {
                sum = sum + this->_data[row_index * this->_cols + inner_index] * other._data[inner_index * other._cols + column_index];
                ++inner_index;
            }
            result._data[row_index * other._cols + column_index] = sum;
            ++column_index;
        }
        ++row_index;
    }
    other._mutex.unlock(THREAD_ID);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::transpose() const
{
    ft_matrix<ElementType> result;
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (result);
    if (!result.init(this->_cols, this->_rows))
    {
        this->set_error(MATRIX_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    size_t row_index = 0;
    while (row_index < this->_rows)
    {
        size_t column_index = 0;
        while (column_index < this->_cols)
        {
            result._data[column_index * this->_rows + row_index] = this->_data[row_index * this->_cols + column_index];
            ++column_index;
        }
        ++row_index;
    }
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
ElementType ft_matrix<ElementType>::determinant() const
{
    ElementType det = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (det);
    if (this->_rows != this->_cols)
    {
        this->set_error(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (det);
    }
    size_t n = this->_rows;
    size_t size = n * n;
    ElementType* temp = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * size));
    if (temp == ft_nullptr)
    {
        this->set_error(MATRIX_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return (det);
    }
    size_t index = 0;
    while (index < size)
    {
        temp[index] = this->_data[index];
        ++index;
    }
    ElementType result = ElementType();
    result = result + 1; // initialize to 1
    size_t pivot_row = 0;
    while (pivot_row < n)
    {
        size_t pivot = pivot_row;
        while (pivot < n && temp[pivot * n + pivot_row] == ElementType())
            ++pivot;
        if (pivot == n)
        {
            result = ElementType();
            break;
        }
        if (pivot != pivot_row)
        {
            size_t swap_index = 0;
            while (swap_index < n)
            {
                ElementType tmp = temp[pivot_row * n + swap_index];
                temp[pivot_row * n + swap_index] = temp[pivot * n + swap_index];
                temp[pivot * n + swap_index] = tmp;
                ++swap_index;
            }
            result = result * static_cast<ElementType>(-1);
        }
        ElementType pivotVal = temp[pivot_row * n + pivot_row];
        result = result * pivotVal;
        size_t row_below = pivot_row + 1;
        while (row_below < n)
        {
            ElementType factor = temp[row_below * n + pivot_row] / pivotVal;
            size_t column = pivot_row;
            while (column < n)
            {
                temp[row_below * n + column] = temp[row_below * n + column] - factor * temp[pivot_row * n + column];
                ++column;
            }
            ++row_below;
        }
        ++pivot_row;
    }
    cma_free(temp);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
int ft_matrix<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_matrix<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_matrix<ElementType>::clear()
{
    if (this->_data != ft_nullptr)
    {
        size_t total = this->_rows * this->_cols;
        size_t i = 0;
        while (i < total)
        {
            destroy_at(&this->_data[i]);
            ++i;
        }
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_rows = 0;
    this->_cols = 0;
    return ;
}

template <typename ElementType>
void ft_matrix<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    return ;
}

#endif // FT_MATRIX_HPP
