#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>

template <typename ValueType>
class Iterator
{
    private:
        ValueType                 *_ptr;
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
            pf_printf_fd(2, "Iterator lifecycle error: %s: %s\n", method_name, reason);
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
            lock_result = this->_mutex->lock();
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
            unlock_result = this->_mutex->unlock();
            if (unlock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_result));
            return (FT_ERR_SUCCESS);
        }

        static ValueType &fallback_reference() noexcept
        {
            static ValueType fallback = ValueType();

            return (fallback);
        }

    public:
        class reference_proxy
        {
            private:
                ValueType *_pointer;
                int32_t    _error;

            public:
                reference_proxy(ValueType *pointer, int32_t error) noexcept
                    : _pointer(pointer), _error(error)
                {
                    return ;
                }

                operator ValueType&() const noexcept
                {
                    if (this->_pointer == ft_nullptr)
                        return (Iterator<ValueType>::fallback_reference());
                    return (*this->_pointer);
                }

                ValueType *operator->() const noexcept
                {
                    return (this->_pointer);
                }

                int32_t get_error() const noexcept
                {
                    return (this->_error);
                }
        };

        Iterator() noexcept
            : _ptr(ft_nullptr), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        explicit Iterator(ValueType *pointer) noexcept
            : _ptr(ft_nullptr), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(pointer);
            return ;
        }

        ~Iterator()
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("Iterator::~Iterator",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        Iterator(const Iterator &other) = delete;
        Iterator(Iterator &&other) = delete;
        Iterator &operator=(const Iterator &other) = delete;
        Iterator &operator=(Iterator &&other) = delete;

        int initialize() noexcept
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("Iterator::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_ptr = ft_nullptr;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(ValueType *pointer) noexcept
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("Iterator::initialize(pointer)",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_ptr = pointer;
            this->_initialized_state = _state_initialized;
            if (pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_INVALID_ARGUMENT));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy() noexcept
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
            {
                this->abort_lifecycle_error("Iterator::destroy",
                    "called while object is not initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->_ptr = ft_nullptr;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        Iterator& operator++() noexcept
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("Iterator::operator++");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (*this);
            }
            if (this->_ptr == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (*this);
            }
            ++this->_ptr;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                set_last_operation_error(unlock_error);
            else
                set_last_operation_error(FT_ERR_SUCCESS);
            return (*this);
        }

        bool operator!=(const Iterator& other) const noexcept
        {
            bool this_lock_acquired;
            bool other_lock_acquired;
            int lock_error;
            int unlock_error;
            bool is_different;

            this->abort_if_not_initialized("Iterator::operator!=");
            other.abort_if_not_initialized("Iterator::operator!= other");
            this_lock_acquired = false;
            lock_error = this->lock_internal(&this_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            other_lock_acquired = false;
            lock_error = other.lock_internal(&other_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                unlock_error = this->unlock_internal(this_lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(lock_error);
                return (false);
            }
            is_different = (this->_ptr != other._ptr);
            unlock_error = other.unlock_internal(other_lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                this->unlock_internal(this_lock_acquired);
                set_last_operation_error(unlock_error);
                return (false);
            }
            unlock_error = this->unlock_internal(this_lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (is_different);
        }

        reference_proxy operator*() const noexcept
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            ValueType *pointer_value;

            this->abort_if_not_initialized("Iterator::operator*");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (reference_proxy(ft_nullptr,
                    set_last_operation_error(lock_error)));
            pointer_value = this->_ptr;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (reference_proxy(ft_nullptr,
                    set_last_operation_error(unlock_error)));
            if (pointer_value == ft_nullptr)
                return (reference_proxy(ft_nullptr,
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT)));
            return (reference_proxy(pointer_value,
                set_last_operation_error(FT_ERR_SUCCESS)));
        }

        ValueType *operator->() const noexcept
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            ValueType *pointer_value;

            this->abort_if_not_initialized("Iterator::operator->");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (ft_nullptr);
            }
            pointer_value = this->_ptr;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (ft_nullptr);
            }
            if (pointer_value == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (pointer_value);
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("Iterator::enable_thread_safety");
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

        bool is_thread_safe() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("Iterator::lock");
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

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }

        static const char *last_operation_error_str() noexcept
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

template <typename ValueType>
thread_local int32_t Iterator<ValueType>::_last_error = FT_ERR_SUCCESS;

#endif
