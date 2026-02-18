#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstddef>
#include "move.hpp"

template <typename ElementType>
class ft_stack
{
    private:
        struct StackNode
        {
            ElementType _data;
            StackNode* _next;
        };

        StackNode* _top;
        size_t _size;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        void destroy_all_unlocked()
        {
            StackNode *node;

            while (this->_top != ft_nullptr)
            {
                node = this->_top;
                this->_top = node->_next;
                destroy_at(&node->_data);
                cma_free(node);
            }
            this->_size = 0;
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (this->_mutex->lock() != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            if (lock_acquired == false || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (this->_mutex->unlock() != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_UNLOCK_FAILED));
            return (FT_ERR_SUCCESS);
        }

    public:
        ft_stack()
            : _top(ft_nullptr), _size(0), _mutex(ft_nullptr)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_stack()
        {
            this->clear();
            (void)this->disable_thread_safety();
            return ;
        }

        ft_stack(const ft_stack&) = delete;
        ft_stack& operator=(const ft_stack&) = delete;
        ft_stack(ft_stack&& other) = delete;
        ft_stack& operator=(ft_stack&& other) = delete;

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
            int result;

            result = this->lock_internal(lock_acquired);
            if (result != FT_ERR_SUCCESS)
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
            StackNode *new_node;
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
            if (new_node == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            construct_at(&new_node->_data, value);
            new_node->_next = this->_top;
            this->_top = new_node;
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void push(ElementType&& value)
        {
            StackNode *new_node;
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
            if (new_node == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            construct_at(&new_node->_data, ft_move(value));
            new_node->_next = this->_top;
            this->_top = new_node;
            this->_size += 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        ElementType pop()
        {
            bool lock_acquired;
            StackNode *node;
            ElementType value;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (ElementType());
            if (this->_top == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (ElementType());
            }
            node = this->_top;
            this->_top = node->_next;
            value = ft_move(node->_data);
            destroy_at(&node->_data);
            cma_free(node);
            this->_size -= 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (value);
        }

        ElementType& top()
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;
            ElementType *value;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (error_element);
            if (this->_top == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (error_element);
            }
            value = &this->_top->_data;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (*value);
        }

        const ElementType& top() const
        {
            static ElementType error_element = ElementType();
            bool lock_acquired;
            int lock_error;
            const ElementType *value;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (error_element);
            if (this->_top == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_EMPTY);
                (void)this->unlock_internal(lock_acquired);
                return (error_element);
            }
            value = &this->_top->_data;
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
            bool lock_acquired;
            int lock_error;
            bool is_empty;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (true);
            is_empty = (this->_top == ft_nullptr);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (is_empty);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_error;

            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return ;
            this->destroy_all_unlocked();
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

template <typename ElementType>
thread_local int32_t ft_stack<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
