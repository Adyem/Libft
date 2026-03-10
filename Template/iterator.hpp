#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
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
        uint8_t                    _initialised_state;

        static const uint8_t _state_uninitialised = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialised = 2;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept
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

        void abort_if_not_initialised(const char *method_name) const
        {
            if (this->_initialised_state == _state_initialised)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialised");
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
              _initialised_state(_state_uninitialised)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        explicit Iterator(ValueType *pointer) noexcept
            : _ptr(ft_nullptr), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            (void)this->initialize(pointer);
            return ;
        }

        ~Iterator()
        {
            if (this->_initialised_state == _state_initialised)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        Iterator(const Iterator &other) = delete;
        Iterator(Iterator &&other) = delete;
        Iterator &operator=(const Iterator &other) = delete;
        Iterator &operator=(Iterator &&other) = delete;

        int32_t initialize() noexcept
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("Iterator::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_ptr = ft_nullptr;
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(ValueType *pointer) noexcept
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("Iterator::initialize(pointer)",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_ptr = pointer;
            this->_initialised_state = _state_initialised;
            if (pointer == ft_nullptr)
                return (set_error(FT_ERR_INVALID_ARGUMENT));
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t destroy() noexcept
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            if (this->_initialised_state != _state_initialised)
                return (set_error(FT_ERR_SUCCESS));
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            this->_ptr = ft_nullptr;
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        Iterator& operator++() noexcept
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("Iterator::operator++");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (*this);
            }
            if (this->_ptr == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_INVALID_ARGUMENT);
                return (*this);
            }
            ++this->_ptr;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (*this);
        }

        bool operator!=(const Iterator& other) const noexcept
        {
            ft_bool this_lock_acquired;
            ft_bool other_lock_acquired;
            int32_t lock_error;
            bool is_different;

            this->abort_if_not_initialised("Iterator::operator!=");
            other.abort_if_not_initialised("Iterator::operator!= other");
            this_lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&this_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (false);
            }
            other_lock_acquired = FT_FALSE;
            lock_error = other.lock_internal(&other_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(this_lock_acquired);
                set_error(lock_error);
                return (false);
            }
            is_different = (this->_ptr != other._ptr);
            (void)other.unlock_internal(other_lock_acquired);
            (void)this->unlock_internal(this_lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (is_different);
        }

        reference_proxy operator*() const noexcept
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            ValueType *pointer_value;

            this->abort_if_not_initialised("Iterator::operator*");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (reference_proxy(ft_nullptr,
                    set_error(lock_error)));
            pointer_value = this->_ptr;
            (void)this->unlock_internal(lock_acquired);
            if (pointer_value == ft_nullptr)
                return (reference_proxy(ft_nullptr,
                    set_error(FT_ERR_INVALID_ARGUMENT)));
            return (reference_proxy(pointer_value,
                set_error(FT_ERR_SUCCESS)));
        }

        ValueType *operator->() const noexcept
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            ValueType *pointer_value;

            this->abort_if_not_initialised("Iterator::operator->");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (ft_nullptr);
            }
            pointer_value = this->_ptr;
            (void)this->unlock_internal(lock_acquired);
            if (pointer_value == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            set_error(FT_ERR_SUCCESS);
            return (pointer_value);
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("Iterator::enable_thread_safety");
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

            if (this->_initialised_state != _state_initialised
                && this->_initialised_state != _state_destroyed)
                return (set_error(FT_ERR_INVALID_STATE));
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

        bool is_thread_safe() const noexcept
        {
            if (this->_initialised_state != _state_initialised)
            {
                set_error(FT_ERR_INVALID_STATE);
                return (false);
            }
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("Iterator::lock");
            lock_result = this->lock_internal(lock_acquired);
            return (set_error(lock_result));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
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

template <typename ValueType>
thread_local int32_t Iterator<ValueType>::_last_error = FT_ERR_SUCCESS;

#endif
