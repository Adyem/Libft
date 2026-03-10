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
        ft_size_t                     _size;
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
            pf_printf_fd(2, "ft_string_view lifecycle error: %s: %s\n", method_name,
                reason);
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

    public:
        static const ft_size_t npos;

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
              _initialised_state(_state_uninitialised)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ft_string_view(const CharType* string)
            : _data(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            (void)this->initialize(string);
            return ;
        }

        ft_string_view(const CharType* string, ft_size_t size)
            : _data(ft_nullptr), _size(0), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            (void)this->initialize(string, size);
            return ;
        }

        ~ft_string_view()
        {
            if (this->_initialised_state == _state_initialised)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_string_view(const ft_string_view& other) = delete;
        ft_string_view& operator=(const ft_string_view& other) = delete;
        ft_string_view(ft_string_view&& other) = delete;
        ft_string_view& operator=(ft_string_view&& other) = delete;

        int32_t initialize()
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_string_view::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_data = ft_nullptr;
            this->_size = 0;
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(const CharType *string)
        {
            ft_size_t index;

            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_string_view::initialize(string)",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
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
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(const CharType *string, ft_size_t size)
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_string_view::initialize(string,size)",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_data = string;
            this->_size = size;
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t destroy()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            if (this->_initialised_state != _state_initialised)
                return (set_error(FT_ERR_SUCCESS));
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            this->_data = ft_nullptr;
            this->_size = 0;
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        const CharType* data() const
        {
            const CharType *pointer_value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::data");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (ft_nullptr);
            }
            pointer_value = this->_data;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (pointer_value);
        }

        ft_size_t size() const
        {
            ft_size_t size_value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::size");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (0);
            }
            size_value = this->_size;
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (size_value);
        }

        bool empty() const
        {
            bool is_empty_value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::empty");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (true);
            }
            is_empty_value = (this->_size == 0);
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (is_empty_value);
        }

        char_proxy operator[](ft_size_t index) const
        {
            CharType char_value;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::operator[]");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (char_proxy(CharType(), set_error(lock_error)));
            if (index >= this->_size || this->_data == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
                return (char_proxy(CharType(),
                    set_error(FT_ERR_INVALID_ARGUMENT)));
            }
            char_value = this->_data[index];
            (void)this->unlock_internal(lock_acquired);
            return (char_proxy(char_value, set_error(FT_ERR_SUCCESS)));
        }

        int32_t compare(const ft_string_view& other) const
        {
            ft_size_t first_index;
            ft_size_t first_size;
            ft_size_t second_size;
            const CharType *first_data;
            const CharType *second_data;
            int32_t compare_result;
            ft_bool first_lock_acquired;
            ft_bool second_lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::compare");
            other.abort_if_not_initialised("ft_string_view::compare other");
            first_lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&first_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            second_lock_acquired = FT_FALSE;
            lock_error = other.lock_internal(&second_lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                (void)this->unlock_internal(first_lock_acquired);
                return (set_error(lock_error));
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
            (void)other.unlock_internal(second_lock_acquired);
            (void)this->unlock_internal(first_lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (compare_result);
        }

        int32_t substr(ft_size_t position, ft_size_t count, ft_string_view& out_view) const
        {
            const CharType *new_data;
            ft_size_t new_count;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_string_view::substr");
            out_view.abort_if_not_initialised("ft_string_view::substr out_view");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                return (set_error(lock_error));
            }
            if (position > this->_size)
            {
                (void)this->unlock_internal(lock_acquired);
                return (set_error(FT_ERR_INVALID_ARGUMENT));
            }
            new_count = this->_size - position;
            if (count != npos && count < new_count)
                new_count = count;
            new_data = this->_data + position;
            out_view._data = new_data;
            out_view._size = new_count;
            (void)this->unlock_internal(lock_acquired);
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t substr(ft_size_t position, ft_string_view& out_view) const
        {
            return (this->substr(position, npos, out_view));
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("ft_string_view::enable_thread_safety");
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

        bool is_thread_safe() const
        {
            this->abort_if_not_initialised("ft_string_view::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("ft_string_view::lock");
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

template <typename CharType>
const ft_size_t ft_string_view<CharType>::npos = static_cast<ft_size_t>(-1);

template <typename CharType>
thread_local int32_t ft_string_view<CharType>::_last_error = FT_ERR_SUCCESS;

#endif
