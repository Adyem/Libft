#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ManagedType>
class ft_sharedptr
{
    private:
        ManagedType                *_managed_pointer;
        size_t                     *_reference_count;
        size_t                      _array_size;
        bool                        _is_array_type;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                     _initialized_state;
        static const uint8_t        _state_uninitialized = 0;
        static const uint8_t        _state_destroyed = 1;
        static const uint8_t        _state_initialized = 2;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        int32_t lock_internal(bool *lock_acquired) const noexcept
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

        void unlock_internal(bool lock_acquired) const noexcept
        {
            if (lock_acquired == false)
                return ;
            if (this->_mutex == ft_nullptr)
                return ;
            (void)this->_mutex->unlock();
            return ;
        }

        void destroy_storage() noexcept
        {
            if (this->_managed_pointer == ft_nullptr)
            {
                this->_reference_count = ft_nullptr;
                return ;
            }
            if (this->_reference_count == ft_nullptr)
            {
                if (this->_is_array_type)
                    delete[] this->_managed_pointer;
                else
                    delete this->_managed_pointer;
                this->_managed_pointer = ft_nullptr;
                this->_array_size = 0;
                this->_is_array_type = false;
                return ;
            }
            if (*this->_reference_count > 1)
            {
                *this->_reference_count = *this->_reference_count - 1;
                this->_managed_pointer = ft_nullptr;
                this->_reference_count = ft_nullptr;
                this->_array_size = 0;
                this->_is_array_type = false;
                return ;
            }
            if (this->_is_array_type)
                delete[] this->_managed_pointer;
            else
                delete this->_managed_pointer;
            delete this->_reference_count;
            this->_managed_pointer = ft_nullptr;
            this->_reference_count = ft_nullptr;
            this->_array_size = 0;
            this->_is_array_type = false;
            return ;
        }

    public:
        class reference_proxy
        {
            private:
                ManagedType *_pointer;
                int32_t      _error;

            public:
                reference_proxy(ManagedType *pointer, int32_t error) noexcept
                    : _pointer(pointer), _error(error)
                {
                    return ;
                }

                operator ManagedType&() const noexcept
                {
                    static ManagedType fallback = ManagedType();

                    if (this->_pointer == ft_nullptr)
                        return (fallback);
                    return (*this->_pointer);
                }

                ManagedType *operator->() const noexcept
                {
                    return (this->_pointer);
                }

                int32_t get_error() const noexcept
                {
                    return (this->_error);
                }
        };

        class const_reference_proxy
        {
            private:
                const ManagedType *_pointer;
                int32_t            _error;

            public:
                const_reference_proxy(const ManagedType *pointer,
                    int32_t error) noexcept
                    : _pointer(pointer), _error(error)
                {
                    return ;
                }

                operator const ManagedType&() const noexcept
                {
                    static ManagedType fallback = ManagedType();

                    if (this->_pointer == ft_nullptr)
                        return (fallback);
                    return (*this->_pointer);
                }

                const ManagedType *operator->() const noexcept
                {
                    return (this->_pointer);
                }

                int32_t get_error() const noexcept
                {
                    return (this->_error);
                }
        };

        ft_sharedptr() noexcept
            : _managed_pointer(ft_nullptr), _reference_count(ft_nullptr),
              _array_size(0), _is_array_type(false),
              _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_sharedptr(ManagedType *pointer, bool array_type = false,
                size_t array_size = 1) noexcept
            : _managed_pointer(ft_nullptr), _reference_count(ft_nullptr),
              _array_size(0), _is_array_type(false),
              _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(pointer, array_type, array_size);
            return ;
        }

        explicit ft_sharedptr(size_t size) noexcept
            : _managed_pointer(ft_nullptr), _reference_count(ft_nullptr),
              _array_size(0), _is_array_type(false),
              _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(size);
            return ;
        }

        ft_sharedptr(const ft_sharedptr &other) noexcept
            : _managed_pointer(ft_nullptr), _reference_count(ft_nullptr),
              _array_size(0), _is_array_type(false),
              _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            if (other._initialized_state == _state_initialized)
            {
                this->_managed_pointer = other._managed_pointer;
                this->_reference_count = other._reference_count;
                this->_array_size = other._array_size;
                this->_is_array_type = other._is_array_type;
                this->_initialized_state = _state_initialized;
                if (this->_reference_count != ft_nullptr)
                    *this->_reference_count = *this->_reference_count + 1;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_sharedptr(ft_sharedptr &&other) noexcept
            : _managed_pointer(ft_nullptr), _reference_count(ft_nullptr),
              _array_size(0), _is_array_type(false),
              _mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            if (other._initialized_state == _state_initialized)
            {
                this->_managed_pointer = other._managed_pointer;
                this->_reference_count = other._reference_count;
                this->_array_size = other._array_size;
                this->_is_array_type = other._is_array_type;
                this->_initialized_state = _state_initialized;
                other._managed_pointer = ft_nullptr;
                other._reference_count = ft_nullptr;
                other._array_size = 0;
                other._is_array_type = false;
                other._initialized_state = _state_destroyed;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_sharedptr &operator=(const ft_sharedptr &other) noexcept
        {
            if (this == &other)
                return (*this);
            if (this->_initialized_state != _state_initialized)
                (void)this->initialize();
            this->destroy_storage();
            if (other._initialized_state != _state_initialized)
                return (*this);
            this->_managed_pointer = other._managed_pointer;
            this->_reference_count = other._reference_count;
            this->_array_size = other._array_size;
            this->_is_array_type = other._is_array_type;
            if (this->_reference_count != ft_nullptr)
                *this->_reference_count = *this->_reference_count + 1;
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*this);
        }

        ft_sharedptr &operator=(ft_sharedptr &&other) noexcept
        {
            if (this == &other)
                return (*this);
            if (this->_initialized_state != _state_initialized)
                (void)this->initialize();
            this->destroy_storage();
            this->_managed_pointer = other._managed_pointer;
            this->_reference_count = other._reference_count;
            this->_array_size = other._array_size;
            this->_is_array_type = other._is_array_type;
            other._managed_pointer = ft_nullptr;
            other._reference_count = ft_nullptr;
            other._array_size = 0;
            other._is_array_type = false;
            set_last_operation_error(FT_ERR_SUCCESS);
            return (*this);
        }

        ~ft_sharedptr()
        {
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        int32_t initialize() noexcept
        {
            if (this->_initialized_state == _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            this->_managed_pointer = ft_nullptr;
            this->_reference_count = ft_nullptr;
            this->_array_size = 0;
            this->_is_array_type = false;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(ManagedType *pointer, bool array_type = false,
                size_t array_size = 1) noexcept
        {
            if (this->_initialized_state == _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            this->_managed_pointer = pointer;
            this->_reference_count = ft_nullptr;
            if (pointer != ft_nullptr)
            {
                this->_reference_count = new (std::nothrow) size_t(1);
                if (this->_reference_count == ft_nullptr)
                {
                    this->_initialized_state = _state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
            }
            this->_array_size = array_type ? array_size : (pointer == ft_nullptr ? 0 : 1);
            this->_is_array_type = array_type;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(size_t size) noexcept
        {
            if (this->_initialized_state == _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            this->_managed_pointer = ft_nullptr;
            this->_reference_count = ft_nullptr;
            this->_array_size = 0;
            this->_is_array_type = true;
            if (size > 0)
            {
                this->_managed_pointer = new (std::nothrow) ManagedType[size];
                if (this->_managed_pointer == ft_nullptr)
                {
                    this->_initialized_state = _state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
            }
            this->_array_size = size;
            if (this->_managed_pointer != ft_nullptr)
            {
                this->_reference_count = new (std::nothrow) size_t(1);
                if (this->_reference_count == ft_nullptr)
                {
                    delete[] this->_managed_pointer;
                    this->_managed_pointer = ft_nullptr;
                    this->_array_size = 0;
                    this->_initialized_state = _state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int32_t destroy() noexcept
        {
            bool lock_acquired;
            int32_t lock_result;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (lock_result);
            this->destroy_storage();
            this->_initialized_state = _state_destroyed;
            this->unlock_internal(lock_acquired);
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        reference_proxy operator*() noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_managed_pointer == ft_nullptr)
                return (reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_POINTER)));
            return (reference_proxy(this->_managed_pointer,
                    set_last_operation_error(FT_ERR_SUCCESS)));
        }

        const_reference_proxy operator*() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (const_reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_managed_pointer == ft_nullptr)
                return (const_reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_POINTER)));
            return (const_reference_proxy(this->_managed_pointer,
                    set_last_operation_error(FT_ERR_SUCCESS)));
        }

        ManagedType *operator->() noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            if (this->_managed_pointer == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_POINTER);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer);
        }

        const ManagedType *operator->() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            if (this->_managed_pointer == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_INVALID_POINTER);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer);
        }

        reference_proxy operator[](size_t index) noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_is_array_type == false || this->_managed_pointer == ft_nullptr)
                return (reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_OPERATION)));
            if (index >= this->_array_size)
                return (reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_OUT_OF_RANGE)));
            return (reference_proxy(&this->_managed_pointer[index],
                    set_last_operation_error(FT_ERR_SUCCESS)));
        }

        const_reference_proxy operator[](size_t index) const noexcept
        {
            if (this->_initialized_state != _state_initialized)
                return (const_reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_STATE)));
            if (this->_is_array_type == false || this->_managed_pointer == ft_nullptr)
                return (const_reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_INVALID_OPERATION)));
            if (index >= this->_array_size)
                return (const_reference_proxy(ft_nullptr,
                        set_last_operation_error(FT_ERR_OUT_OF_RANGE)));
            return (const_reference_proxy(&this->_managed_pointer[index],
                    set_last_operation_error(FT_ERR_SUCCESS)));
        }

        ManagedType *get() noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer);
        }

        const ManagedType *get() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer);
        }

        int use_count() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (0);
            }
            if (this->_managed_pointer == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_SUCCESS);
                return (0);
            }
            if (this->_reference_count == ft_nullptr)
                return (1);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (static_cast<int>(*this->_reference_count));
        }

        bool unique() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer != ft_nullptr);
        }

        void reset(ManagedType *pointer = ft_nullptr, size_t size = 1,
                bool array_type = false) noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            this->destroy_storage();
            this->_managed_pointer = pointer;
            this->_reference_count = ft_nullptr;
            if (pointer != ft_nullptr)
            {
                this->_reference_count = new (std::nothrow) size_t(1);
                if (this->_reference_count == ft_nullptr)
                {
                    this->_managed_pointer = ft_nullptr;
                    this->_array_size = 0;
                    this->_is_array_type = false;
                    set_last_operation_error(FT_ERR_NO_MEMORY);
                    return ;
                }
            }
            this->_array_size = array_type ? size : (pointer == ft_nullptr ? 0 : 1);
            this->_is_array_type = array_type;
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void swap(ft_sharedptr &other) noexcept
        {
            ManagedType *pointer_value;
            size_t array_size_value;
            bool is_array_type_value;

            if (this->_initialized_state != _state_initialized
                || other._initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            pointer_value = this->_managed_pointer;
            size_t *reference_count_value;
            array_size_value = this->_array_size;
            is_array_type_value = this->_is_array_type;
            reference_count_value = this->_reference_count;
            this->_managed_pointer = other._managed_pointer;
            this->_reference_count = other._reference_count;
            this->_array_size = other._array_size;
            this->_is_array_type = other._is_array_type;
            other._managed_pointer = pointer_value;
            other._reference_count = reference_count_value;
            other._array_size = array_size_value;
            other._is_array_type = is_array_type_value;
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        explicit operator bool() const noexcept
        {
            if (this->_initialized_state != _state_initialized)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return (false);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_managed_pointer != ft_nullptr);
        }

        int32_t enable_thread_safety() noexcept
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

        int32_t disable_thread_safety() noexcept
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

        bool is_thread_safe() const noexcept
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const noexcept
        {
            int32_t lock_result;

            lock_result = this->lock_internal(lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }

        void unlock(bool lock_acquired) const noexcept
        {
            this->unlock_internal(lock_acquired);
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

template <typename ManagedType>
thread_local int32_t ft_sharedptr<ManagedType>::_last_error = FT_ERR_SUCCESS;

template <typename LeftType, typename RightType>
bool operator==(const ft_sharedptr<LeftType> &left, const ft_sharedptr<RightType> &right)
{
    return (left.get() == right.get());
}

template <typename LeftType, typename RightType>
bool operator!=(const ft_sharedptr<LeftType> &left, const ft_sharedptr<RightType> &right)
{
    return (left.get() != right.get());
}

#endif
