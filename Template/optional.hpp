#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

template <typename ElementType>
class ft_optional
{
    private:
        ElementType               *_value;
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
            pf_printf_fd(2, "ft_optional lifecycle error: %s: %s\n", method_name, reason);
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

        static ElementType &fallback_reference() noexcept
        {
            if constexpr (std::is_default_constructible_v<ElementType>)
            {
                static ElementType fallback = ElementType();
                return (fallback);
            }
            else
            {
                alignas(ElementType) static unsigned char storage[sizeof(ElementType)] = {0};
                return (*reinterpret_cast<ElementType*>(storage));
            }
        }

        void destroy_value_unlocked()
        {
            if (this->_value == ft_nullptr)
                return ;
            destroy_at(this->_value);
            cma_free(this->_value);
            this->_value = ft_nullptr;
            return ;
        }

    public:
        class value_proxy
        {
            private:
                ElementType *_element_pointer;
                int32_t      _error;

            public:
                value_proxy(ElementType *element_pointer, int32_t error) noexcept
                    : _element_pointer(element_pointer), _error(error)
                {
                    return ;
                }

                operator ElementType&() const noexcept
                {
                    if (this->_element_pointer == ft_nullptr)
                        return (ft_optional<ElementType>::fallback_reference());
                    return (*this->_element_pointer);
                }

                ElementType *operator->() const noexcept
                {
                    return (this->_element_pointer);
                }

                int32_t get_error() const noexcept
                {
                    return (this->_error);
                }
        };

        class const_value_proxy
        {
            private:
                const ElementType *_element_pointer;
                int32_t            _error;

            public:
                const_value_proxy(const ElementType *element_pointer,
                    int32_t error) noexcept
                    : _element_pointer(element_pointer), _error(error)
                {
                    return ;
                }

                operator const ElementType&() const noexcept
                {
                    if (this->_element_pointer == ft_nullptr)
                        return (ft_optional<ElementType>::fallback_reference());
                    return (*this->_element_pointer);
                }

                const ElementType *operator->() const noexcept
                {
                    return (this->_element_pointer);
                }

                int32_t get_error() const noexcept
                {
                    return (this->_error);
                }
        };

        ft_optional()
            : _value(ft_nullptr), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_optional(const ElementType& value)
            : _value(ft_nullptr), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(value);
            return ;
        }

        ft_optional(ElementType&& value)
            : _value(ft_nullptr), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(ft_move(value));
            return ;
        }

        ~ft_optional()
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("ft_optional::~ft_optional",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_optional(const ft_optional &other) = delete;
        ft_optional(ft_optional &&other) = delete;
        ft_optional &operator=(const ft_optional &other) = delete;
        ft_optional &operator=(ft_optional &&other) = delete;

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_optional::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_value = ft_nullptr;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(const ElementType& value)
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_optional::initialize(copy)",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_value = static_cast<ElementType*>(cma_malloc(sizeof(ElementType)));
            if (this->_value == ft_nullptr)
            {
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            }
            construct_at(this->_value, value);
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(ElementType&& value)
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_optional::initialize(move)",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_value = static_cast<ElementType*>(cma_malloc(sizeof(ElementType)));
            if (this->_value == ft_nullptr)
            {
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            }
            construct_at(this->_value, ft_move(value));
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
            {
                this->abort_lifecycle_error("ft_optional::destroy",
                    "called while object is not initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->destroy_value_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool has_value() const
        {
            bool has_stored_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_optional::has_value");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (false);
            }
            has_stored_value = (this->_value != ft_nullptr);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (has_stored_value);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (has_stored_value);
        }

        ElementType& value()
        {
            static ElementType fallback = ElementType();
            ElementType *value_pointer;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_optional::value");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (fallback);
            }
            if (this->_value == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_EMPTY);
                return (fallback);
            }
            value_pointer = this->_value;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (fallback);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        const ElementType& value() const
        {
            static ElementType fallback = ElementType();
            const ElementType *value_pointer;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_optional::value const");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (fallback);
            }
            if (this->_value == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_EMPTY);
                return (fallback);
            }
            value_pointer = this->_value;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (fallback);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*value_pointer);
        }

        value_proxy operator*() noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (value_proxy(ft_nullptr,
                    set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_value == ft_nullptr)
                return (value_proxy(ft_nullptr,
                    set_last_operation_error(FT_ERR_EMPTY)));
            return (value_proxy(this->_value,
                set_last_operation_error(FT_ERR_SUCCESS)));
        }

        const_value_proxy operator*() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (const_value_proxy(ft_nullptr,
                    set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_value == ft_nullptr)
                return (const_value_proxy(ft_nullptr,
                    set_last_operation_error(FT_ERR_EMPTY)));
            return (const_value_proxy(this->_value,
                set_last_operation_error(FT_ERR_SUCCESS)));
        }

        ElementType *operator->() noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            if (this->_value == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_EMPTY);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_value);
        }

        const ElementType *operator->() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            if (this->_value == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_EMPTY);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_value);
        }

        explicit operator bool() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_value != ft_nullptr);
        }

        void reset()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_optional::reset");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->destroy_value_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int enable_thread_safety() noexcept
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
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

        int disable_thread_safety() noexcept
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

template <typename ElementType>
thread_local int32_t ft_optional<ElementType>::_last_error = FT_ERR_SUCCESS;

#endif
