#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "swap.hpp"
#include <cstddef>
#include <functional>
#include "move.hpp"

template <typename ElementType, typename Compare = std::less<ElementType> >
class ft_priority_queue
{
    private:
        ElementType*                 _data;
        size_t                       _capacity;
        size_t                       _size;
        Compare                      _comp;
        mutable pt_recursive_mutex*  _mutex;
        static thread_local int32_t  _last_error;

        static int32_t set_last_operation_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        bool ensure_capacity(size_t desired)
        {
            size_t new_capacity;
            ElementType* new_data;
            size_t element_index;

            if (desired <= this->_capacity)
            {
                set_last_operation_error(FT_ERR_SUCCESS);
                return (true);
            }
            if (this->_capacity == 0)
                new_capacity = 1;
            else
                new_capacity = this->_capacity * 2;
            while (new_capacity < desired)
                new_capacity *= 2;
            new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
            if (new_data == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
                return (false);
            }
            element_index = 0;
            while (element_index < this->_size)
            {
                ::construct_at(&new_data[element_index], ft_move(this->_data[element_index]));
                ::destroy_at(&this->_data[element_index]);
                element_index += 1;
            }
            if (this->_data != ft_nullptr)
                cma_free(this->_data);
            this->_data = new_data;
            this->_capacity = new_capacity;
            set_last_operation_error(FT_ERR_SUCCESS);
            return (true);
        }

        void heapify_up(size_t index)
        {
            size_t parent_index;

            while (index > 0)
            {
                parent_index = (index - 1) / 2;
                if (!this->_comp(this->_data[parent_index], this->_data[index]))
                    break ;
                ft_swap(this->_data[parent_index], this->_data[index]);
                index = parent_index;
            }
            return ;
        }

        void heapify_down(size_t index)
        {
            size_t left_child_index;
            size_t right_child_index;
            size_t largest_index;

            while (true)
            {
                left_child_index = index * 2 + 1;
                right_child_index = left_child_index + 1;
                largest_index = index;
                if (left_child_index < this->_size
                    && this->_comp(this->_data[largest_index], this->_data[left_child_index]))
                    largest_index = left_child_index;
                if (right_child_index < this->_size
                    && this->_comp(this->_data[largest_index], this->_data[right_child_index]))
                    largest_index = right_child_index;
                if (largest_index == index)
                    break ;
                ft_swap(this->_data[index], this->_data[largest_index]);
                index = largest_index;
            }
            return ;
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
        ft_priority_queue(size_t initial_capacity = 0,
                const Compare& comp = Compare())
            : _data(ft_nullptr), _capacity(0), _size(0), _comp(comp), _mutex(ft_nullptr)
        {
            if (initial_capacity > 0)
            {
                this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initial_capacity));
                if (this->_data == ft_nullptr)
                {
                    set_last_operation_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
                    return ;
                }
                this->_capacity = initial_capacity;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_priority_queue()
        {
            this->clear();
            if (this->_data != ft_nullptr)
                cma_free(this->_data);
            (void)this->disable_thread_safety();
            return ;
        }

        ft_priority_queue(const ft_priority_queue&) = delete;
        ft_priority_queue& operator=(const ft_priority_queue&) = delete;
        ft_priority_queue(ft_priority_queue&& other) = delete;
        ft_priority_queue& operator=(ft_priority_queue&& other) = delete;

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

        void push(const ElementType& value)
        {
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            if (!this->ensure_capacity(this->_size + 1))
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            ::construct_at(&this->_data[this->_size], value);
            this->heapify_up(this->_size);
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void push(ElementType&& value)
        {
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            if (!this->ensure_capacity(this->_size + 1))
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            ::construct_at(&this->_data[this->_size], ft_move(value));
            this->heapify_up(this->_size);
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        ElementType pop()
        {
            bool lock_acquired;
            int lock_error;
            ElementType value;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (ElementType());
            if (this->_size == 0)
            {
                set_last_operation_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (ElementType());
            }
            value = ft_move(this->_data[0]);
            ::destroy_at(&this->_data[0]);
            this->_size -= 1;
            if (this->_size > 0)
            {
                ::construct_at(&this->_data[0], ft_move(this->_data[this->_size]));
                ::destroy_at(&this->_data[this->_size]);
                this->heapify_down(0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (value);
        }

        ElementType& top()
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (error_element);
            if (this->_size == 0)
            {
                set_last_operation_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (error_element);
            }
            ElementType *value = &this->_data[0];
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (*value);
        }

        const ElementType& top() const
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (error_element);
            if (this->_size == 0)
            {
                set_last_operation_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (error_element);
            }
            const ElementType *value = &this->_data[0];
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (*value);
        }

        size_t size() const
        {
            size_t current_size;
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (0);
            current_size = this->_size;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (current_size);
        }

        bool empty() const
        {
            bool result;
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (true);
            result = (this->_size == 0);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        void clear()
        {
            size_t element_index;
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            element_index = 0;
            while (element_index < this->_size)
            {
                ::destroy_at(&this->_data[element_index]);
                element_index += 1;
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
        pt_recursive_mutex* get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename ElementType, typename Compare>
thread_local int32_t ft_priority_queue<ElementType, Compare>::_last_error = FT_ERR_SUCCESS;

#endif
