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
        mutable int    _errorCode;
        mutable pt_mutex _mutex;

        void    setError(int error) const;

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
    : _data(ft_nullptr), _rows(0), _cols(0), _errorCode(ER_SUCCESS)
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
    : _data(other._data), _rows(other._rows), _cols(other._cols), _errorCode(other._errorCode)
{
    other._data = ft_nullptr;
    other._rows = 0;
    other._cols = 0;
    other._errorCode = ER_SUCCESS;
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
        this->_errorCode = other._errorCode;
        other._data = ft_nullptr;
        other._rows = 0;
        other._cols = 0;
        other._errorCode = ER_SUCCESS;
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
        this->setError(MATRIX_ALLOC_FAIL);
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
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (errorElement);
    if (r >= this->_rows || c >= this->_cols)
    {
        this->setError(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[r * this->_cols + c];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_matrix<ElementType>::at(size_t r, size_t c) const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (errorElement);
    if (r >= this->_rows || c >= this->_cols)
    {
        this->setError(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[r * this->_cols + c];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
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
        this->setError(MATRIX_DIM_MISMATCH);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (!result.init(this->_rows, this->_cols))
    {
        this->setError(MATRIX_ALLOC_FAIL);
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
        this->setError(MATRIX_DIM_MISMATCH);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    if (!result.init(this->_rows, other._cols))
    {
        this->setError(MATRIX_ALLOC_FAIL);
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    size_t i = 0;
    while (i < this->_rows)
    {
        size_t j = 0;
        while (j < other._cols)
        {
            ElementType sum = ElementType();
            size_t k = 0;
            while (k < this->_cols)
            {
                sum = sum + this->_data[i * this->_cols + k] * other._data[k * other._cols + j];
                ++k;
            }
            result._data[i * other._cols + j] = sum;
            ++j;
        }
        ++i;
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
        this->setError(MATRIX_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return (result);
    }
    size_t i = 0;
    while (i < this->_rows)
    {
        size_t j = 0;
        while (j < this->_cols)
        {
            result._data[j * this->_rows + i] = this->_data[i * this->_cols + j];
            ++j;
        }
        ++i;
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
        this->setError(MATRIX_DIM_MISMATCH);
        this->_mutex.unlock(THREAD_ID);
        return (det);
    }
    size_t n = this->_rows;
    size_t size = n * n;
    ElementType* temp = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * size));
    if (temp == ft_nullptr)
    {
        this->setError(MATRIX_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return (det);
    }
    size_t idx = 0;
    while (idx < size)
    {
        temp[idx] = this->_data[idx];
        ++idx;
    }
    ElementType result = ElementType();
    result = result + 1; // initialize to 1
    size_t i = 0;
    while (i < n)
    {
        size_t pivot = i;
        while (pivot < n && temp[pivot * n + i] == ElementType())
            ++pivot;
        if (pivot == n)
        {
            result = ElementType();
            break;
        }
        if (pivot != i)
        {
            size_t k = 0;
            while (k < n)
            {
                ElementType tmp = temp[i * n + k];
                temp[i * n + k] = temp[pivot * n + k];
                temp[pivot * n + k] = tmp;
                ++k;
            }
            result = result * static_cast<ElementType>(-1);
        }
        ElementType pivotVal = temp[i * n + i];
        result = result * pivotVal;
        size_t j = i + 1;
        while (j < n)
        {
            ElementType factor = temp[j * n + i] / pivotVal;
            size_t k = i;
            while (k < n)
            {
                temp[j * n + k] = temp[j * n + k] - factor * temp[i * n + k];
                ++k;
            }
            ++j;
        }
        ++i;
    }
    cma_free(temp);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
int ft_matrix<ElementType>::get_error() const
{
    return (this->_errorCode);
}

template <typename ElementType>
const char* ft_matrix<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_errorCode));
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
void ft_matrix<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    return ;
}

#endif // FT_MATRIX_HPP
