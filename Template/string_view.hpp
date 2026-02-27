#ifndef FT_STRING_VIEW_HPP
#define FT_STRING_VIEW_HPP

#include <cstddef>
#include <cstdint>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

template <typename CharType>
class ft_string_view
{
    private:
        const CharType            *_data;
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
            pf_printf_fd(2, "ft_string_view lifecycle error: %s: %s\n", method_name,
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

    public:
        static const size_t npos;

        class char_proxy
        {
            private:
                CharType _value;
                int32_t  _error;

            public:
                char_proxy(CharType value, int32_t error)
                    : _value(value), _error(error)
                {
                    return ;
                }

                operator CharType() const
                {
                    return (this->_value);
                }

                int32_t get_error() const
                {
                    return (this->_error);
                }
        };

        ft_string_view()
            : _data(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_string_view(const CharType* string)
            : _data(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(string);
            return ;
        }

        ft_string_view(const CharType* string, size_t size)
            : _data(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            (void)this->initialize(string, size);
            return ;
        }

        ~ft_string_view()
        {
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_string_view(const ft_string_view& other) = delete;
        ft_string_view& operator=(const ft_string_view& other) = delete;
        ft_string_view(ft_string_view&& other) = delete;
        ft_string_view& operator=(ft_string_view&& other) = delete;

        int initialize()
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_string_view::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_data = ft_nullptr;
            this->_size = 0;
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(const CharType *string)
        {
            size_t index;

            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_string_view::initialize(string)",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_data = string;
            this->_size = 0;
            if (string != ft_nullptr)
            {
                index = 0;
                while (string[index] != CharType())
                    ++index;
                this->_size = index;
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(const CharType *string, size_t size)
        {
            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_string_view::initialize(string,size)",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_data = string;
            this->_size = size;
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
            this->_data = ft_nullptr;
            this->_size = 0;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        const CharType* data() const
        {
            const CharType *pointer_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::data");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (ft_nullptr);
            }
            pointer_value = this->_data;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (ft_nullptr);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (pointer_value);
        }

        size_t size() const
        {
            size_t size_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::size");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (0);
            }
            size_value = this->_size;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (0);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (size_value);
        }

        bool empty() const
        {
            bool is_empty_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::empty");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return (true);
            }
            is_empty_value = (this->_size == 0);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return (true);
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return (is_empty_value);
        }

        char_proxy operator[](size_t index) const
        {
            CharType char_value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::operator[]");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (char_proxy(CharType(), set_last_operation_error(lock_error)));
            if (index >= this->_size || this->_data == ft_nullptr)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    return (char_proxy(CharType(), set_last_operation_error(unlock_error)));
                return (char_proxy(CharType(),
                    set_last_operation_error(FT_ERR_INVALID_ARGUMENT)));
            }
            char_value = this->_data[index];
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (char_proxy(CharType(), set_last_operation_error(unlock_error)));
            return (char_proxy(char_value, set_last_operation_error(FT_ERR_SUCCESS)));
        }

        int compare(const ft_string_view& other) const
        {
            size_t first_index;
            size_t first_size;
            size_t second_size;
            const CharType *first_data;
            const CharType *second_data;
            int compare_result;
            bool first_lock_acquired;
            bool second_lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::compare");
            other.abort_if_not_initialized("ft_string_view::compare other");
            first_lock_acquired = false;
            lock_error = this->lock_internal(&first_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            second_lock_acquired = false;
            lock_error = other.lock_internal(&second_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(first_lock_acquired);
                return (set_last_operation_error(lock_error));
            }
            first_data = this->_data;
            second_data = other._data;
            first_size = this->_size;
            second_size = other._size;
            first_index = 0;
            compare_result = 0;
            while (first_index < first_size && first_index < second_size)
            {
                if (first_data[first_index] != second_data[first_index])
                {
                    if (first_data[first_index] < second_data[first_index])
                        compare_result = -1;
                    else
                        compare_result = 1;
                    break ;
                }
                ++first_index;
            }
            if (compare_result == 0)
            {
                if (first_size < second_size)
                    compare_result = -1;
                else if (first_size > second_size)
                    compare_result = 1;
            }
            unlock_error = other.unlock_internal(second_lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(first_lock_acquired);
                return (set_last_operation_error(unlock_error));
            }
            unlock_error = this->unlock_internal(first_lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            set_last_operation_error(FT_ERR_SUCCESS);
            return (compare_result);
        }

        int substr(size_t position, size_t count, ft_string_view& out_view) const
        {
            const CharType *new_data;
            size_t new_count;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_string_view::substr");
            out_view.abort_if_not_initialized("ft_string_view::substr out_view");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                return (set_last_operation_error(lock_error));
            }
            if (position > this->_size)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    return (set_last_operation_error(unlock_error));
                return (set_last_operation_error(FT_ERR_INVALID_ARGUMENT));
            }
            new_count = this->_size - position;
            if (count != npos && count < new_count)
                new_count = count;
            new_data = this->_data + position;
            out_view._data = new_data;
            out_view._size = new_count;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int substr(size_t position, ft_string_view& out_view) const
        {
            return (this->substr(position, npos, out_view));
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_string_view::enable_thread_safety");
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
            this->abort_if_not_initialized("ft_string_view::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("ft_string_view::lock");
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
        pt_recursive_mutex* get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename CharType>
const size_t ft_string_view<CharType>::npos = static_cast<size_t>(-1);

template <typename CharType>
thread_local int32_t ft_string_view<CharType>::_last_error = FT_ERR_SUCCESS;

#endif
