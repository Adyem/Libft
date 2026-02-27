#ifndef FT_SET_HPP
#define FT_SET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>
#include "move.hpp"

template <typename ElementType>
class ft_set
{
    private:
        ElementType*                  _data;
        size_t                        _capacity;
        size_t                        _size;
        mutable pt_recursive_mutex*   _mutex;
        static thread_local int32_t   _last_error;

        static int32_t set_last_operation_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        bool ensure_capacity(size_t desired_capacity)
        {
            size_t new_capacity;
            ElementType* new_data;
            size_t index;

            if (desired_capacity <= this->_capacity)
                return (true);
            if (this->_capacity == 0)
                new_capacity = 1;
            else
                new_capacity = this->_capacity * 2;
            while (new_capacity < desired_capacity)
                new_capacity *= 2;
            new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
            if (new_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (false);
            }
            index = 0;
            while (index < this->_size)
            {
                ::construct_at(&new_data[index], ft_move(this->_data[index]));
                ::destroy_at(&this->_data[index]);
                index += 1;
            }
            if (this->_data != ft_nullptr)
                cma_free(this->_data);
            this->_data = new_data;
            this->_capacity = new_capacity;
            return (true);
        }

        size_t find_index(const ElementType& value) const
        {
            size_t left;
            size_t right;
            size_t middle;

            left = 0;
            right = this->_size;
            while (left < right)
            {
                middle = left + (right - left) / 2;
                if (this->_data[middle] < value)
                    left = middle + 1;
                else if (value < this->_data[middle])
                    right = middle;
                else
                    return (middle);
            }
            return (this->_size);
        }

        size_t lower_bound(const ElementType& value) const
        {
            size_t left;
            size_t right;
            size_t middle;

            left = 0;
            right = this->_size;
            while (left < right)
            {
                middle = left + (right - left) / 2;
                if (this->_data[middle] < value)
                    left = middle + 1;
                else
                    right = middle;
            }
            return (left);
        }

        int lock_internal(bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            if (lock_acquired == false || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_unlock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_UNLOCK_FAILED));
            return (FT_ERR_SUCCESS);
        }

    public:
        ft_set(size_t initial_capacity = 0)
            : _data(ft_nullptr), _capacity(0), _size(0), _mutex(ft_nullptr)
        {
            if (initial_capacity > 0)
            {
                this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initial_capacity));
                if (this->_data == ft_nullptr)
                {
                    set_last_operation_error(FT_ERR_NO_MEMORY);
                    return ;
                }
                this->_capacity = initial_capacity;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_set()
        {
            this->clear();
            if (this->_data != ft_nullptr)
                cma_free(this->_data);
            (void)this->disable_thread_safety();
            return ;
        }

        ft_set(const ft_set&) = delete;
        ft_set& operator=(const ft_set&) = delete;
        ft_set(ft_set&& other) = delete;
        ft_set& operator=(ft_set&& other) = delete;

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            if (this->_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_last_operation_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int destroy_result;

            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(destroy_result));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            if (this->lock_internal(lock_acquired) != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void insert(const ElementType& value)
        {
            bool lock_acquired;
            int lock_result;
            size_t position;
            size_t index;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            position = this->lower_bound(value);
            if (position < this->_size && !(value < this->_data[position])
                && !(this->_data[position] < value))
            {
                set_last_operation_error(FT_ERR_SUCCESS);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            if (!this->ensure_capacity(this->_size + 1))
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            index = this->_size;
            while (index > position)
            {
                ::construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
                ::destroy_at(&this->_data[index - 1]);
                index -= 1;
            }
            ::construct_at(&this->_data[position], value);
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void insert(ElementType&& value)
        {
            bool lock_acquired;
            int lock_result;
            size_t position;
            size_t index;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            position = this->lower_bound(value);
            if (position < this->_size && !(value < this->_data[position])
                && !(this->_data[position] < value))
            {
                set_last_operation_error(FT_ERR_SUCCESS);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            if (!this->ensure_capacity(this->_size + 1))
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            index = this->_size;
            while (index > position)
            {
                ::construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
                ::destroy_at(&this->_data[index - 1]);
                index -= 1;
            }
            ::construct_at(&this->_data[position], ft_move(value));
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        ElementType* find(const ElementType& value)
        {
            bool lock_acquired;
            int lock_result;
            size_t index;
            ElementType *result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (ft_nullptr);
            index = this->find_index(value);
            if (index == this->_size)
            {
                set_last_operation_error(FT_ERR_NOT_FOUND);
                (void)this->unlock_internal(lock_acquired);
                return (ft_nullptr);
            }
            result = &this->_data[index];
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        const ElementType* find(const ElementType& value) const
        {
            bool lock_acquired;
            int lock_result;
            size_t index;
            const ElementType *result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (ft_nullptr);
            index = this->find_index(value);
            if (index == this->_size)
            {
                set_last_operation_error(FT_ERR_NOT_FOUND);
                (void)this->unlock_internal(lock_acquired);
                return (ft_nullptr);
            }
            result = &this->_data[index];
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        void remove(const ElementType& value)
        {
            bool lock_acquired;
            int lock_result;
            size_t index;
            size_t current_index;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            index = this->find_index(value);
            if (index == this->_size)
            {
                set_last_operation_error(FT_ERR_NOT_FOUND);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            ::destroy_at(&this->_data[index]);
            current_index = index;
            while (current_index + 1 < this->_size)
            {
                ::construct_at(&this->_data[current_index], ft_move(this->_data[current_index + 1]));
                ::destroy_at(&this->_data[current_index + 1]);
                current_index += 1;
            }
            this->_size -= 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        size_t size() const
        {
            bool lock_acquired;
            int lock_result;
            size_t current_size;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (0);
            current_size = this->_size;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (current_size);
        }

        bool empty() const
        {
            bool lock_acquired;
            int lock_result;
            bool result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (true);
            result = (this->_size == 0);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_result;
            size_t index;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            index = 0;
            while (index < this->_size)
            {
                ::destroy_at(&this->_data[index]);
                index += 1;
            }
            this->_size = 0;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t last_operation_error()
        {
            return (_last_error);
        }

        static const char *last_operation_error_str()
        {
            return (ft_strerror(_last_error));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename ElementType>
thread_local int32_t ft_set<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
