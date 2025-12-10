#ifndef FT_MATRIX_HPP
#define FT_MATRIX_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>

template <typename ElementType>
class ft_matrix
{
    private:
        ElementType*   _data;
        size_t         _rows;
        size_t         _cols;
        mutable int    _error_code;
        mutable pt_mutex    *_mutex;
        bool                _thread_safe_enabled;

        void    set_error(int error) const;
        void    clear_unlocked();
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
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

        int get_error() const;
        const char* get_error_str() const;
        void clear();
};

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(size_t rows, size_t cols)
    : _data(ft_nullptr),
      _rows(0),
      _cols(0),
      _error_code(FT_ER_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (rows > 0 && cols > 0)
    {
        if (!this->init(rows, cols))
            return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>::~ft_matrix()
{
    this->clear();
    this->teardown_thread_safety();
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>::ft_matrix(ft_matrix&& other) noexcept
    : _data(ft_nullptr),
      _rows(0),
      _cols(0),
      _error_code(FT_ER_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    bool other_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    this->_data = other._data;
    this->_rows = other._rows;
    this->_cols = other._cols;
    this->_error_code = other._error_code;
    other._data = ft_nullptr;
    other._rows = 0;
    other._cols = 0;
    other._error_code = FT_ER_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    other._thread_safe_enabled = false;
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_matrix<ElementType>& ft_matrix<ElementType>::operator=(ft_matrix&& other) noexcept
{
    if (this != &other)
    {
        bool this_lock_acquired;
        bool other_lock_acquired;
        bool other_thread_safe;

        this_lock_acquired = false;
        if (this->lock_internal(&this_lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        other_lock_acquired = false;
        if (other.lock_internal(&other_lock_acquired) != 0)
        {
            this->unlock_internal(this_lock_acquired);
            this->set_error(ft_errno);
            return (*this);
        }
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        this->clear_unlocked();
        this->_data = other._data;
        this->_rows = other._rows;
        this->_cols = other._cols;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._rows = 0;
        other._cols = 0;
        other._error_code = FT_ER_SUCCESSS;
        other.unlock_internal(other_lock_acquired);
        other.teardown_thread_safety();
        other._thread_safe_enabled = false;
        this->unlock_internal(this_lock_acquired);
        this->teardown_thread_safety();
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
            {
                other.set_error(FT_ER_SUCCESSS);
                return (*this);
            }
        }
        other.set_error(FT_ER_SUCCESSS);
        this->set_error(this->_error_code);
        return (*this);
    }
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

template <typename ElementType>
int ft_matrix<ElementType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ER_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ER_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ER_SUCCESSS);
    return (0);
}

template <typename ElementType>
void ft_matrix<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_matrix<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_matrix<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
    else
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_matrix<ElementType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ER_SUCCESSS)
        const_cast<ft_matrix<ElementType> *>(this)->set_error(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
    }
    return ;
}

template <typename ElementType>
bool ft_matrix<ElementType>::init(size_t rows, size_t cols)
{
    bool lock_acquired;
    size_t total;
    ElementType *allocated_data;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (false);
    }
    total = rows * cols;
    this->clear_unlocked();
    if (total == 0)
    {
        this->_rows = rows;
        this->_cols = cols;
        this->set_error(FT_ER_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return (true);
    }
    allocated_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * total));
    if (allocated_data == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return (false);
    }
    index = 0;
    while (index < total)
    {
        construct_default_at(&allocated_data[index]);
        ++index;
    }
    this->_data = allocated_data;
    this->_rows = rows;
    this->_cols = cols;
    this->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (true);
}

template <typename ElementType>
ElementType& ft_matrix<ElementType>::at(size_t r, size_t c)
{
    static ElementType error_element = ElementType();
    bool lock_acquired;
    ElementType *element_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (error_element);
    }
    if (r >= this->_rows || c >= this->_cols)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    element_pointer = &this->_data[r * this->_cols + c];
    this->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*element_pointer);
}

template <typename ElementType>
const ElementType& ft_matrix<ElementType>::at(size_t r, size_t c) const
{
    static ElementType error_element = ElementType();
    bool lock_acquired;
    const ElementType *element_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (error_element);
    }
    if (r >= this->_rows || c >= this->_cols)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    element_pointer = &this->_data[r * this->_cols + c];
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*element_pointer);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::rows() const
{
    bool   lock_acquired;
    size_t current_rows;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_rows = this->_rows;
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_rows);
}

template <typename ElementType>
size_t ft_matrix<ElementType>::cols() const
{
    bool   lock_acquired;
    size_t current_cols;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_cols = this->_cols;
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_cols);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::add(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    bool this_lock_acquired;
    bool other_lock_acquired;
    size_t total;
    size_t index;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (result);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    if (this->_rows != other._rows || this->_cols != other._cols)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        const_cast<ft_matrix<ElementType> &>(other).set_error(FT_ERR_INVALID_ARGUMENT);
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    if (!result.init(this->_rows, this->_cols))
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(result.get_error());
        const_cast<ft_matrix<ElementType> &>(other).set_error(result.get_error());
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    total = this->_rows * this->_cols;
    index = 0;
    while (index < total)
    {
        result._data[index] = this->_data[index] + other._data[index];
        ++index;
    }
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    const_cast<ft_matrix<ElementType> &>(other).set_error(FT_ER_SUCCESSS);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::multiply(const ft_matrix& other) const
{
    ft_matrix<ElementType> result;
    bool this_lock_acquired;
    bool other_lock_acquired;
    size_t row_index;
    size_t column_index;
    size_t inner_index;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (result);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    if (this->_cols != other._rows)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        const_cast<ft_matrix<ElementType> &>(other).set_error(FT_ERR_INVALID_ARGUMENT);
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    if (!result.init(this->_rows, other._cols))
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(result.get_error());
        const_cast<ft_matrix<ElementType> &>(other).set_error(result.get_error());
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        return (result);
    }
    row_index = 0;
    while (row_index < this->_rows)
    {
        column_index = 0;
        while (column_index < other._cols)
        {
            ElementType sum = ElementType();
            inner_index = 0;
            while (inner_index < this->_cols)
            {
                sum = sum + this->_data[row_index * this->_cols + inner_index] *
                    other._data[inner_index * other._cols + column_index];
                ++inner_index;
            }
            result._data[row_index * other._cols + column_index] = sum;
            ++column_index;
        }
        ++row_index;
    }
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    const_cast<ft_matrix<ElementType> &>(other).set_error(FT_ER_SUCCESSS);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (result);
}

template <typename ElementType>
ft_matrix<ElementType> ft_matrix<ElementType>::transpose() const
{
    ft_matrix<ElementType> result;
    bool lock_acquired;
    size_t row_index;
    size_t column_index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (result);
    }
    if (!result.init(this->_cols, this->_rows))
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(result.get_error());
        this->unlock_internal(lock_acquired);
        return (result);
    }
    row_index = 0;
    while (row_index < this->_rows)
    {
        column_index = 0;
        while (column_index < this->_cols)
        {
            result._data[column_index * this->_rows + row_index] =
                this->_data[row_index * this->_cols + column_index];
            ++column_index;
        }
        ++row_index;
    }
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
ElementType ft_matrix<ElementType>::determinant() const
{
    ElementType det = ElementType();
    bool lock_acquired;
    size_t n;
    size_t size;
    ElementType *temp;
    size_t index;
    ElementType result;
    size_t pivot_row;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(ft_errno);
        return (det);
    }
    if (this->_rows != this->_cols)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (det);
    }
    n = this->_rows;
    size = n * n;
    temp = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * size));
    if (temp == ft_nullptr)
    {
        const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return (det);
    }
    index = 0;
    while (index < size)
    {
        temp[index] = this->_data[index];
        ++index;
    }
    result = ElementType();
    result = result + 1;
    pivot_row = 0;
    while (pivot_row < n)
    {
        size_t pivot;

        pivot = pivot_row;
        while (pivot < n && temp[pivot * n + pivot_row] == ElementType())
            ++pivot;
        if (pivot == n)
        {
            result = ElementType();
            break;
        }
        if (pivot != pivot_row)
        {
            size_t swap_index;

            swap_index = 0;
            while (swap_index < n)
            {
                ElementType tmp = temp[pivot_row * n + swap_index];
                temp[pivot_row * n + swap_index] = temp[pivot * n + swap_index];
                temp[pivot * n + swap_index] = tmp;
                ++swap_index;
            }
            result = result * static_cast<ElementType>(-1);
        }
        ElementType pivot_value;

        pivot_value = temp[pivot_row * n + pivot_row];
        result = result * pivot_value;
        size_t row_below;

        row_below = pivot_row + 1;
        while (row_below < n)
        {
            ElementType factor;
            size_t column;

            factor = temp[row_below * n + pivot_row] / pivot_value;
            column = pivot_row;
            while (column < n)
            {
                temp[row_below * n + column] = temp[row_below * n + column] -
                    factor * temp[pivot_row * n + column];
                ++column;
            }
            ++row_below;
        }
        ++pivot_row;
    }
    cma_free(temp);
    const_cast<ft_matrix<ElementType> *>(this)->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
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
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->clear_unlocked();
    this->set_error(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_matrix<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_matrix<ElementType>::clear_unlocked()
{
    if (this->_data != ft_nullptr)
    {
        size_t total;
        size_t i;

        total = this->_rows * this->_cols;
        i = 0;
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
int ft_matrix<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ER_SUCCESSS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = FT_ER_SUCCESSS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

template <typename ElementType>
void ft_matrix<ElementType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ElementType>
void ft_matrix<ElementType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
