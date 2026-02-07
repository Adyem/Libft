#ifndef FT_MATRIX_HPP
#define FT_MATRIX_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>

template <typename ElementType>
class ft_matrix
{
    private:
        ElementType*            _data;
        size_t                  _rows;
        size_t                  _cols;
        mutable pt_recursive_mutex* _mutex;

        void    clear_unlocked();
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        ft_matrix(size_t rows = 0, size_t cols = 0);
        ~ft_matrix();

        ft_matrix(const ft_matrix&) = delete;
        ft_matrix& operator=(const ft_matrix&) = delete;

        ft_matrix(ft_matrix&& other) noexcept;
        ft_matrix& operator=(ft_matrix&& other) noexcept;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        bool    init(size_t rows, size_t cols);
        ElementType& at(size_t r, size_t c);
        const ElementType& at(size_t r, size_t c) const;
        size_t  rows() const;
        size_t  cols() const;

        ft_matrix add(const ft_matrix& other) const;
        ft_matrix multiply(const ft_matrix& other) const;
        ft_matrix transpose() const;
        ElementType determinant() const;

        void clear();

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(size_t rows, size_t cols)
    : _data(ft_nullptr), _rows(0), _cols(0), _mutex(ft_nullptr)
{
    if (rows > 0 && cols > 0)
        this->init(rows, cols);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ElementType>
ft_matrix<ElementType>::~ft_matrix()
{
    clear();
    teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(ft_matrix&& other) noexcept
    : _data(ft_nullptr), _rows(0), _cols(0), _mutex(ft_nullptr)
{
    bool has_lock = false;
    int lock_result = other.lock_internal(&has_lock);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    _data = other._data;
    _rows = other._rows;
    _cols = other._cols;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._data = ft_nullptr;
    other._rows = 0;
    other._cols = 0;
    other.unlock_internal(has_lock);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ElementType>
ft_matrix<ElementType>& ft_matrix<ElementType>::operator=(ft_matrix&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);

    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (*this);
    }
    bool other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);

    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (*this);
    }
    clear_unlocked();
    this->_data = other._data;
    this->_rows = other._rows;
    this->_cols = other._cols;
    other._data = ft_nullptr;
    other._rows = 0;
    other._cols = 0;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    teardown_thread_safety();
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ElementType>
int ft_matrix<ElementType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    int result = prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType>
void ft_matrix<ElementType>::disable_thread_safety()
{
    teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ElementType>
bool ft_matrix<ElementType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_matrix<ElementType>::lock(bool *lock_acquired) const
{
    int result = lock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

template <typename ElementType>
void ft_matrix<ElementType>::unlock(bool lock_acquired) const
{
    int result = unlock_internal(lock_acquired);
    ft_global_error_stack_push(result);
}

template <typename ElementType>
bool ft_matrix<ElementType>::init(size_t rows, size_t cols)
{
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (false);
    }
    size_t total = rows * cols;
    clear_unlocked();
    if (total == 0)
    {
        _rows = rows;
        _cols = cols;
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (true);
    }
    ElementType *allocated_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * total));
    if (allocated_data == ft_nullptr)
    {
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t index = 0;
    while (index < total)
    {
        construct_default_at(&allocated_data[index]);
        ++index;
    }
    _data = allocated_data;
    _rows = rows;
    _cols = cols;
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

template <typename ElementType>
ElementType& ft_matrix<ElementType>::at(size_t r, size_t c)
{
    static ElementType error_element = ElementType();
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (error_element);
    }
    if (r >= _rows || c >= _cols || _data == ft_nullptr)
    {
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (error_element);
    }
    ElementType& element = _data[r * _cols + c];
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (element);
}

template <typename ElementType>
const ElementType& ft_matrix<ElementType>::at(size_t r, size_t c) const
{
    static ElementType error_element = ElementType();
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (error_element);
    }
    if (r >= _rows || c >= _cols || _data == ft_nullptr)
    {
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (error_element);
    }
    const ElementType& element = _data[r * _cols + c];
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (element);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::rows() const
{
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    size_t current_rows = _rows;
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (current_rows);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::cols() const
{
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (0);
    }
    size_t current_cols = _cols;
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (current_cols);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::add(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    bool this_lock_acquired = false;
    int this_lock_result = lock_internal(&this_lock_acquired);

    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (result);
    }
    bool other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);

    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (result);
    }
    if (_rows != other._rows || _cols != other._cols)
    {
        unlock_internal(this_lock_acquired);
        other.unlock_internal(other_lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    if (!result.init(_rows, _cols))
    {
        unlock_internal(this_lock_acquired);
        other.unlock_internal(other_lock_acquired);
        return (result);
    }
    size_t total = _rows * _cols;
    for (size_t index = 0; index < total; ++index)
        result._data[index] = _data[index] + other._data[index];
    unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::multiply(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    bool this_lock_acquired = false;
    int this_lock_result = lock_internal(&this_lock_acquired);

    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (result);
    }
    bool other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);

    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (result);
    }
    if (_cols != other._rows)
    {
        unlock_internal(this_lock_acquired);
        other.unlock_internal(other_lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    if (!result.init(_rows, other._cols))
    {
        unlock_internal(this_lock_acquired);
        other.unlock_internal(other_lock_acquired);
        return (result);
    }
    for (size_t row = 0; row < _rows; ++row)
    {
        for (size_t col = 0; col < other._cols; ++col)
        {
            ElementType sum = ElementType();
            for (size_t inner = 0; inner < _cols; ++inner)
                sum = sum + _data[row * _cols + inner] * other._data[inner * other._cols + col];
            result._data[row * other._cols + col] = sum;
        }
    }
    unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::transpose() const
{
    ft_matrix<ElementType> result;
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (result);
    }
    if (!result.init(_cols, _rows))
    {
        unlock_internal(lock_acquired);
        return (result);
    }
    for (size_t row = 0; row < _rows; ++row)
    {
        for (size_t col = 0; col < _cols; ++col)
        {
            result._data[col * _rows + row] = _data[row * _cols + col];
        }
    }
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
ElementType ft_matrix<ElementType>::determinant() const
{
    ElementType det = ElementType();
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (det);
    }
    if (_rows != _cols)
    {
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (det);
    }
    size_t n = _rows;
    size_t size = n * n;
    ElementType *temp = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * size));
    if (temp == ft_nullptr)
    {
        unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (det);
    }
    for (size_t index = 0; index < size; ++index)
        temp[index] = _data[index];
    ElementType result = ElementType();
    result = result + 1;
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
            for (size_t swap_index = 0; swap_index < n; ++swap_index)
            {
                ElementType tmp = temp[pivot_row * n + swap_index];
                temp[pivot_row * n + swap_index] = temp[pivot * n + swap_index];
                temp[pivot * n + swap_index] = tmp;
            }
            result = result * static_cast<ElementType>(-1);
        }
        ElementType pivot_value = temp[pivot_row * n + pivot_row];
        result = result * pivot_value;
        for (size_t row = pivot_row + 1; row < n; ++row)
        {
            ElementType factor = temp[row * n + pivot_row] / pivot_value;
            for (size_t column = pivot_row; column < n; ++column)
            {
                temp[row * n + column] = temp[row * n + column] -
                    factor * temp[pivot_row * n + column];
            }
        }
        ++pivot_row;
    }
    cma_free(temp);
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_matrix<ElementType>::clear()
{
    bool lock_acquired = false;
    int lock_result = lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    clear_unlocked();
    unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ElementType>
void ft_matrix<ElementType>::clear_unlocked()
{
    if (_data != ft_nullptr)
    {
        size_t total = _rows * _cols;
        for (size_t i = 0; i < total; ++i)
            destroy_at(&_data[i]);
        cma_free(_data);
        _data = ft_nullptr;
    }
    _rows = 0;
    _cols = 0;
}

template <typename ElementType>
int ft_matrix<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_lock_with_error(*_mutex);
    if (result == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template <typename ElementType>
int ft_matrix<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || _mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*_mutex));
}

template <typename ElementType>
int ft_matrix<ElementType>::prepare_thread_safety()
{
    if (_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_create_with_error(&_mutex);
    if (result != FT_ERR_SUCCESSS && _mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&_mutex);
    return (result);
}

template <typename ElementType>
void ft_matrix<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&_mutex);
}

#ifdef LIBFT_TEST_BUILD

template <typename ElementType>
pt_recursive_mutex* ft_matrix<ElementType>::get_mutex_for_validation() const noexcept
{
    return (_mutex);
}
#endif

#endif
