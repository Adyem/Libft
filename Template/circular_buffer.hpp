#ifndef FT_CIRCULAR_BUFFER_HPP
#define FT_CIRCULAR_BUFFER_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ElementType>
class ft_circular_buffer
{
    private:
        ElementType               *_buffer;
        size_t                     _configured_capacity;
        size_t                     _head;
        size_t                     _tail;
        size_t                     _size;
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
            pf_printf_fd(2, "ft_circular_buffer lifecycle error: %s: %s\n",
                method_name, reason);
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

        void destroy_elements_locked()
        {
            size_t index;

            if (this->_configured_capacity == 0)
                return ;
            index = 0;
            while (index < this->_size)
            {
                destroy_at(&this->_buffer[(this->_head + index) % this->_configured_capacity]);
                ++index;
            }
            return ;
        }

        void release_buffer_locked()
        {
            if (this->_buffer != ft_nullptr)
            {
                cma_free(this->_buffer);
                this->_buffer = ft_nullptr;
            }
            return ;
        }

    public:
        explicit ft_circular_buffer(size_t capacity)
            : _buffer(ft_nullptr), _configured_capacity(capacity), _head(0), _tail(0),
              _size(0), _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_circular_buffer()
        {
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_circular_buffer(const ft_circular_buffer&) = delete;
        ft_circular_buffer& operator=(const ft_circular_buffer&) = delete;
        ft_circular_buffer(ft_circular_buffer&& other) = delete;
        ft_circular_buffer& operator=(ft_circular_buffer&& other) = delete;

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_circular_buffer::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_buffer = ft_nullptr;
            this->_head = 0;
            this->_tail = 0;
            this->_size = 0;
            if (this->_configured_capacity > 0)
            {
                this->_buffer = static_cast<ElementType*>(cma_malloc(sizeof(ElementType)
                        * this->_configured_capacity));
                if (this->_buffer == ft_nullptr)
                {
                    this->_initialized_state = _state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->destroy_elements_locked();
            this->release_buffer_locked();
            this->_head = 0;
            this->_tail = 0;
            this->_size = 0;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        void push(const ElementType& value)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::push(copy)");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (this->_size == this->_configured_capacity)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_FULL);
                return ;
            }
            construct_at(&this->_buffer[this->_tail], value);
            if (this->_configured_capacity != 0)
                this->_tail = (this->_tail + 1) % this->_configured_capacity;
            this->_size += 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void push(ElementType&& value)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::push(move)");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (this->_size == this->_configured_capacity)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_FULL);
                return ;
            }
            construct_at(&this->_buffer[this->_tail], ft_move(value));
            if (this->_configured_capacity != 0)
                this->_tail = (this->_tail + 1) % this->_configured_capacity;
            this->_size += 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ElementType pop()
        {
            ElementType value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::pop");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (ElementType());
            }
            if (this->_size == 0)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_EMPTY);
                return (ElementType());
            }
            value = ft_move(this->_buffer[this->_head]);
            destroy_at(&this->_buffer[this->_head]);
            if (this->_configured_capacity != 0)
                this->_head = (this->_head + 1) % this->_configured_capacity;
            this->_size -= 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (ElementType());
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        bool is_full() const
        {
            bool value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::is_full");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            value = (this->_size == this->_configured_capacity);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        bool is_empty() const
        {
            bool value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::is_empty");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (true);
            }
            value = (this->_size == 0);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (true);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        size_t size() const
        {
            size_t value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::size");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            value = this->_size;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        size_t capacity() const
        {
            size_t value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::capacity");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            value = this->_configured_capacity;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_circular_buffer::enable_thread_safety");
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

            if (this->_initialized_state != _state_initialized
                && this->_initialized_state != _state_destroyed)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
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
            this->abort_if_not_initialized("ft_circular_buffer::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("ft_circular_buffer::lock");
            lock_result = this->lock_internal(lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (-1);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void clear()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_circular_buffer::clear");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->destroy_elements_locked();
            this->_head = 0;
            this->_tail = 0;
            this->_size = 0;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }

        static const char *last_operation_error_str() noexcept
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
thread_local int32_t ft_circular_buffer<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
