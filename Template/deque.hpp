#ifndef FT_DEQUE_HPP
#define FT_DEQUE_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ElementType>
class ft_deque
{
    private:
        struct deque_node
        {
            ElementType _data;
            deque_node  *_prev;
            deque_node  *_next;
        };

        deque_node                *_front;
        deque_node                *_back;
        ft_size_t                     _size;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialised_state;

        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            errno_abort_lifecycle(this->_initialised_state, method_name, reason);
            return ;
        }

        void abort_if_not_initialised(const char *method_name) const
        {
            errno_abort_if_uninitialised(this->_initialised_state, method_name);
            return ;
        }

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        int32_t unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            return (FT_ERR_SUCCESS);
        }

        void destroy_all_unlocked()
        {
            deque_node *node;

            while (this->_front != ft_nullptr)
            {
                node = this->_front;
                this->_front = this->_front->_next;
                destroy_at(&node->_data);
                cma_free(node);
            }
            this->_back = ft_nullptr;
            this->_size = 0;
            return ;
        }

    public:
        ft_deque()
            : _front(ft_nullptr), _back(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialised_state(FT_CLASS_STATE_UNINITIALISED)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_deque()
        {
            if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_deque(const ft_deque &other) = delete;
        ft_deque(ft_deque &&other) = delete;
        ft_deque &operator=(const ft_deque &other) = delete;
        ft_deque &operator=(ft_deque &&other) = delete;

        int32_t initialize()
        {
            if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
            {
                this->abort_lifecycle_error("ft_deque::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_front = ft_nullptr;
            this->_back = ft_nullptr;
            this->_size = 0;
            this->_initialised_state = FT_CLASS_STATE_INITIALISED;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t destroy()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
                return (set_error(FT_ERR_SUCCESS));
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            this->destroy_all_unlocked();
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("ft_deque::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int32_t destroy_result;

            this->abort_if_not_initialised("ft_deque::disable_thread_safety");
            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_error(destroy_result));
            return (set_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialised("ft_deque::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("ft_deque::lock");
            lock_result = this->lock_internal(lock_acquired);
            return (set_error(lock_result));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void push_front(const ElementType& value)
        {
            deque_node *new_node;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::push_front");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            new_node = static_cast<deque_node*>(cma_malloc(sizeof(deque_node)));
            if (new_node == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_NO_MEMORY);
                return ;
            }
            construct_at(&new_node->_data, value);
            new_node->_prev = ft_nullptr;
            new_node->_next = this->_front;
            if (this->_front == ft_nullptr)
                this->_back = new_node;
            else
                this->_front->_prev = new_node;
            this->_front = new_node;
            this->_size += 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        void push_front(ElementType&& value)
        {
            deque_node *new_node;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::push_front(move)");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            new_node = static_cast<deque_node*>(cma_malloc(sizeof(deque_node)));
            if (new_node == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_NO_MEMORY);
                return ;
            }
            construct_at(&new_node->_data, ft_move(value));
            new_node->_prev = ft_nullptr;
            new_node->_next = this->_front;
            if (this->_front == ft_nullptr)
                this->_back = new_node;
            else
                this->_front->_prev = new_node;
            this->_front = new_node;
            this->_size += 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        void push_back(const ElementType& value)
        {
            deque_node *new_node;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::push_back");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            new_node = static_cast<deque_node*>(cma_malloc(sizeof(deque_node)));
            if (new_node == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_NO_MEMORY);
                return ;
            }
            construct_at(&new_node->_data, value);
            new_node->_next = ft_nullptr;
            new_node->_prev = this->_back;
            if (this->_back == ft_nullptr)
                this->_front = new_node;
            else
                this->_back->_next = new_node;
            this->_back = new_node;
            this->_size += 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        void push_back(ElementType&& value)
        {
            deque_node *new_node;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::push_back(move)");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            new_node = static_cast<deque_node*>(cma_malloc(sizeof(deque_node)));
            if (new_node == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_NO_MEMORY);
                return ;
            }
            construct_at(&new_node->_data, ft_move(value));
            new_node->_next = ft_nullptr;
            new_node->_prev = this->_back;
            if (this->_back == ft_nullptr)
                this->_front = new_node;
            else
                this->_back->_next = new_node;
            this->_back = new_node;
            this->_size += 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ElementType pop_front()
        {
            deque_node *node;
            ElementType value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::pop_front");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (ElementType());
            }
            if (this->_front == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (ElementType());
            }
            node = this->_front;
            this->_front = node->_next;
            if (this->_front == ft_nullptr)
                this->_back = ft_nullptr;
            else
                this->_front->_prev = ft_nullptr;
            value = ft_move(node->_data);
            destroy_at(&node->_data);
            cma_free(node);
            this->_size -= 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (value);
        }

        ElementType pop_back()
        {
            deque_node *node;
            ElementType value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::pop_back");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (ElementType());
            }
            if (this->_back == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (ElementType());
            }
            node = this->_back;
            this->_back = node->_prev;
            if (this->_back == ft_nullptr)
                this->_front = ft_nullptr;
            else
                this->_back->_next = ft_nullptr;
            value = ft_move(node->_data);
            destroy_at(&node->_data);
            cma_free(node);
            this->_size -= 1;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (value);
        }

        ElementType& front()
        {
            static ElementType error_element = ElementType();
            ElementType *value_pointer;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::front");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (error_element);
            }
            if (this->_front == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (error_element);
            }
            value_pointer = &this->_front->_data;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        const ElementType& front() const
        {
            static ElementType error_element = ElementType();
            const ElementType *value_pointer;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::front const");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (error_element);
            }
            if (this->_front == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (error_element);
            }
            value_pointer = &this->_front->_data;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        ElementType& back()
        {
            static ElementType error_element = ElementType();
            ElementType *value_pointer;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::back");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (error_element);
            }
            if (this->_back == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (error_element);
            }
            value_pointer = &this->_back->_data;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        const ElementType& back() const
        {
            static ElementType error_element = ElementType();
            const ElementType *value_pointer;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::back const");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (error_element);
            }
            if (this->_back == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_EMPTY);
                return (error_element);
            }
            value_pointer = &this->_back->_data;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        ft_size_t size() const
        {
            ft_size_t current_size;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::size");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (0);
            }
            current_size = this->_size;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (current_size);
        }

        bool empty() const
        {
            bool is_empty;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::empty");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (true);
            }
            is_empty = (this->_size == 0);
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (is_empty);
        }

        void clear()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_deque::clear");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            this->destroy_all_unlocked();
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        static int32_t get_error() noexcept
        {
            return (_last_error);
        }

        static const char *get_error_str() noexcept
        {
            return (ft_strerror(_last_error));
        }

};

template <typename ElementType>
thread_local int32_t ft_deque<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
