#ifndef FT_MATRIX_HPP
#define FT_MATRIX_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ElementType>
class ft_matrix
{
    private:
        ElementType               *_data;
        size_t                     _configured_rows;
        size_t                     _configured_cols;
        size_t                     _rows;
        size_t                     _cols;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "ft_matrix lifecycle error: %s: %s\n", method_name,
                reason);
            su_abort();
            return ;
        }

        void abort_if_not_initialized(const char *method_name) const
        {
            if (this->_initialized_state == _state_initialized)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialized");
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            int lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            int unlock_result;

            if (lock_acquired == false)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            unlock_result = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            if (unlock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_result));
            return (FT_ERR_SUCCESS);
        }

        void clear_unlocked()
        {
            size_t index;
            size_t total;

            if (this->_data != ft_nullptr)
            {
                total = this->_rows * this->_cols;
                index = 0;
                while (index < total)
                {
                    destroy_at(&this->_data[index]);
                    index = index + 1;
                }
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            this->_rows = 0;
            this->_cols = 0;
            return ;
        }

        bool init_unlocked(size_t rows, size_t cols)
        {
            size_t total;
            size_t index;

            this->clear_unlocked();
            total = rows * cols;
            if (total == 0)
            {
                this->_rows = rows;
                this->_cols = cols;
                return (true);
            }
            this->_data = static_cast<ElementType *>(cma_malloc(sizeof(ElementType)
                        * total));
            if (this->_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (false);
            }
            index = 0;
            while (index < total)
            {
                construct_default_at(&this->_data[index]);
                index = index + 1;
            }
            this->_rows = rows;
            this->_cols = cols;
            return (true);
        }

    public:
        ft_matrix(size_t rows = 0, size_t cols = 0)
            : _data(ft_nullptr), _configured_rows(rows), _configured_cols(cols),
              _rows(0), _cols(0), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_matrix()
        {
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_matrix(const ft_matrix&) = delete;
        ft_matrix& operator=(const ft_matrix&) = delete;
        ft_matrix(ft_matrix&& other) noexcept
            : _data(other._data), _configured_rows(other._configured_rows),
              _configured_cols(other._configured_cols), _rows(other._rows),
              _cols(other._cols), _mutex(other._mutex),
              _initialized_state(other._initialized_state)
        {
            other._data = ft_nullptr;
            other._configured_rows = 0;
            other._configured_cols = 0;
            other._rows = 0;
            other._cols = 0;
            other._mutex = ft_nullptr;
            other._initialized_state = _state_destroyed;
            return ;
        }

        ft_matrix& operator=(ft_matrix&& other) noexcept
        {
            if (this == &other)
                return (*this);
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            this->_data = other._data;
            this->_configured_rows = other._configured_rows;
            this->_configured_cols = other._configured_cols;
            this->_rows = other._rows;
            this->_cols = other._cols;
            this->_mutex = other._mutex;
            this->_initialized_state = other._initialized_state;
            other._data = ft_nullptr;
            other._configured_rows = 0;
            other._configured_cols = 0;
            other._rows = 0;
            other._cols = 0;
            other._mutex = ft_nullptr;
            other._initialized_state = _state_destroyed;
            return (*this);
        }

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_matrix::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_data = ft_nullptr;
            this->_rows = 0;
            this->_cols = 0;
            if (this->init_unlocked(this->_configured_rows, this->_configured_cols)
                == false)
            {
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(size_t rows, size_t cols)
        {
            this->_configured_rows = rows;
            this->_configured_cols = cols;
            return (this->initialize());
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            int disable_error;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->clear_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            disable_error = this->disable_thread_safety();
            this->_initialized_state = _state_destroyed;
            if (disable_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(disable_error));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int mutex_error;

            this->abort_if_not_initialized("ft_matrix::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            mutex_pointer = new (std::nothrow) pt_recursive_mutex();
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            mutex_error = mutex_pointer->initialize();
            if (mutex_error != FT_ERR_SUCCESS)
            {
                delete mutex_pointer;
                return (set_last_operation_error(mutex_error));
            }
            this->_mutex = mutex_pointer;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            int destroy_error;

            if (this->_initialized_state == _state_initialized)
                this->abort_if_not_initialized("ft_matrix::disable_thread_safety");
            if (this->_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            destroy_error = this->_mutex->destroy();
            delete this->_mutex;
            this->_mutex = ft_nullptr;
            return (set_last_operation_error(destroy_error));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialized("ft_matrix::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_error;

            this->abort_if_not_initialized("ft_matrix::lock");
            lock_error = this->lock_internal(lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        void unlock(bool lock_acquired) const
        {
            int unlock_error;

            this->abort_if_not_initialized("ft_matrix::unlock");
            unlock_error = this->unlock_internal(lock_acquired);
            set_last_operation_error(unlock_error);
            return ;
        }

        bool init(size_t rows, size_t cols)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool init_result;

            this->abort_if_not_initialized("ft_matrix::init");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            init_result = this->init_unlocked(rows, cols);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (false);
            }
            if (init_result == false)
                return (false);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (true);
        }

        ElementType& at(size_t row_index, size_t column_index)
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            ElementType *element_pointer;

            this->abort_if_not_initialized("ft_matrix::at");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (error_element);
            }
            if (row_index >= this->_rows || column_index >= this->_cols
                || this->_data == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (error_element);
            }
            element_pointer = &this->_data[row_index * this->_cols + column_index];
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (error_element);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*element_pointer);
        }

        const ElementType& at(size_t row_index, size_t column_index) const
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            const ElementType *element_pointer;

            this->abort_if_not_initialized("ft_matrix::at const");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (error_element);
            }
            if (row_index >= this->_rows || column_index >= this->_cols
                || this->_data == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (error_element);
            }
            element_pointer = &this->_data[row_index * this->_cols + column_index];
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (error_element);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*element_pointer);
        }

        size_t rows() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t current_rows;

            this->abort_if_not_initialized("ft_matrix::rows");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            current_rows = this->_rows;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (current_rows);
        }

        size_t cols() const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t current_cols;

            this->abort_if_not_initialized("ft_matrix::cols");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            current_cols = this->_cols;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (current_cols);
        }

        ft_matrix add(const ft_matrix& other) const
        {
            ft_matrix result(this->_rows, this->_cols);
            bool this_lock_acquired;
            bool other_lock_acquired;
            int this_lock_error;
            int other_lock_error;
            int this_unlock_error;
            int other_unlock_error;
            size_t total;
            size_t index;

            this->abort_if_not_initialized("ft_matrix::add");
            other.abort_if_not_initialized("ft_matrix::add(other)");
            if (result.initialize() != FT_ERR_SUCCESS)
                return (result);
            this_lock_acquired = false;
            other_lock_acquired = false;
            this_lock_error = this->lock_internal(&this_lock_acquired);
            if (this_lock_error != FT_ERR_SUCCESS)
                return (result);
            other_lock_error = other.lock_internal(&other_lock_acquired);
            if (other_lock_error != FT_ERR_SUCCESS)
            {
                this->unlock_internal(this_lock_acquired);
                return (result);
            }
            if (this->_rows != other._rows || this->_cols != other._cols)
            {
                this->unlock_internal(this_lock_acquired);
                other.unlock_internal(other_lock_acquired);
                set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (result);
            }
            if (result.init(this->_rows, this->_cols) == false)
            {
                this->unlock_internal(this_lock_acquired);
                other.unlock_internal(other_lock_acquired);
                return (result);
            }
            total = this->_rows * this->_cols;
            index = 0;
            while (index < total)
            {
                result._data[index] = this->_data[index] + other._data[index];
                index = index + 1;
            }
            this_unlock_error = this->unlock_internal(this_lock_acquired);
            other_unlock_error = other.unlock_internal(other_lock_acquired);
            if (this_unlock_error != FT_ERR_SUCCESS)
                set_last_operation_error(this_unlock_error);
            else if (other_unlock_error != FT_ERR_SUCCESS)
                set_last_operation_error(other_unlock_error);
            else
                set_last_operation_error(FT_ERR_SUCCESS);
            return (result);
        }

        ft_matrix multiply(const ft_matrix& other) const
        {
            ft_matrix result(this->_rows, other._cols);
            bool this_lock_acquired;
            bool other_lock_acquired;
            int this_lock_error;
            int other_lock_error;
            size_t row_index;
            size_t column_index;
            size_t inner_index;

            this->abort_if_not_initialized("ft_matrix::multiply");
            other.abort_if_not_initialized("ft_matrix::multiply(other)");
            if (result.initialize() != FT_ERR_SUCCESS)
                return (result);
            this_lock_acquired = false;
            other_lock_acquired = false;
            this_lock_error = this->lock_internal(&this_lock_acquired);
            if (this_lock_error != FT_ERR_SUCCESS)
                return (result);
            other_lock_error = other.lock_internal(&other_lock_acquired);
            if (other_lock_error != FT_ERR_SUCCESS)
            {
                this->unlock_internal(this_lock_acquired);
                return (result);
            }
            if (this->_cols != other._rows)
            {
                this->unlock_internal(this_lock_acquired);
                other.unlock_internal(other_lock_acquired);
                set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (result);
            }
            if (result.init(this->_rows, other._cols) == false)
            {
                this->unlock_internal(this_lock_acquired);
                other.unlock_internal(other_lock_acquired);
                return (result);
            }
            row_index = 0;
            while (row_index < this->_rows)
            {
                column_index = 0;
                while (column_index < other._cols)
                {
                    ElementType sum_value;

                    sum_value = ElementType();
                    inner_index = 0;
                    while (inner_index < this->_cols)
                    {
                        sum_value = sum_value + this->_data[row_index * this->_cols + inner_index]
                            * other._data[inner_index * other._cols + column_index];
                        inner_index = inner_index + 1;
                    }
                    result._data[row_index * other._cols + column_index] = sum_value;
                    column_index = column_index + 1;
                }
                row_index = row_index + 1;
            }
            this->unlock_internal(this_lock_acquired);
            other.unlock_internal(other_lock_acquired);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (result);
        }

        ft_matrix transpose() const
        {
            ft_matrix result(this->_cols, this->_rows);
            bool lock_acquired;
            int lock_error;
            size_t row_index;
            size_t column_index;

            this->abort_if_not_initialized("ft_matrix::transpose");
            if (result.initialize() != FT_ERR_SUCCESS)
                return (result);
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (result);
            if (result.init(this->_cols, this->_rows) == false)
            {
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
                    column_index = column_index + 1;
                }
                row_index = row_index + 1;
            }
            this->unlock_internal(lock_acquired);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (result);
        }

        ElementType determinant() const
        {
            ElementType determinant_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t dimension;
            size_t size;
            ElementType *temp;
            size_t index;
            size_t pivot_row;

            this->abort_if_not_initialized("ft_matrix::determinant");
            determinant_value = ElementType();
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (determinant_value);
            }
            if (this->_rows != this->_cols)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (determinant_value);
            }
            dimension = this->_rows;
            size = dimension * dimension;
            temp = static_cast<ElementType *>(cma_malloc(sizeof(ElementType) * size));
            if (temp == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_NO_MEMORY);
                return (determinant_value);
            }
            index = 0;
            while (index < size)
            {
                temp[index] = this->_data[index];
                index = index + 1;
            }
            determinant_value = determinant_value + static_cast<ElementType>(1);
            pivot_row = 0;
            while (pivot_row < dimension)
            {
                size_t pivot_index;

                pivot_index = pivot_row;
                while (pivot_index < dimension
                    && temp[pivot_index * dimension + pivot_row] == ElementType())
                {
                    pivot_index = pivot_index + 1;
                }
                if (pivot_index == dimension)
                {
                    determinant_value = ElementType();
                    break ;
                }
                if (pivot_index != pivot_row)
                {
                    size_t swap_index;

                    swap_index = 0;
                    while (swap_index < dimension)
                    {
                        ElementType swap_value;

                        swap_value = temp[pivot_row * dimension + swap_index];
                        temp[pivot_row * dimension + swap_index] =
                            temp[pivot_index * dimension + swap_index];
                        temp[pivot_index * dimension + swap_index] = swap_value;
                        swap_index = swap_index + 1;
                    }
                    determinant_value = determinant_value * static_cast<ElementType>(-1);
                }
                ElementType pivot_value;
                size_t row_index;

                pivot_value = temp[pivot_row * dimension + pivot_row];
                determinant_value = determinant_value * pivot_value;
                row_index = pivot_row + 1;
                while (row_index < dimension)
                {
                    ElementType factor_value;
                    size_t column_index;

                    factor_value = temp[row_index * dimension + pivot_row] / pivot_value;
                    column_index = pivot_row;
                    while (column_index < dimension)
                    {
                        temp[row_index * dimension + column_index] =
                            temp[row_index * dimension + column_index]
                            - factor_value
                            * temp[pivot_row * dimension + column_index];
                        column_index = column_index + 1;
                    }
                    row_index = row_index + 1;
                }
                pivot_row = pivot_row + 1;
            }
            cma_free(temp);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                set_last_operation_error(unlock_error);
            else
                set_last_operation_error(FT_ERR_SUCCESS);
            return (determinant_value);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_matrix::clear");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->clear_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int get_error() const
        {
            return (ft_matrix<ElementType>::_last_error);
        }

        const char *get_error_str() const
        {
            return (ft_strerror(this->get_error()));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename ElementType>
thread_local int32_t ft_matrix<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
