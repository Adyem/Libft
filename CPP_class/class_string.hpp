#ifndef FT_STRING_HPP
#define FT_STRING_HPP

#include <cstring>
#include <climits>
#include <cstdint>
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/basic.hpp"

class ft_string
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        char*                    _data;
        ft_size_t                _length;
        ft_size_t                _capacity;
        mutable pt_recursive_mutex *_mutex;
        mutable uint8_t            _initialized_state;
        int32_t                    _operation_error;
        static const uint8_t       _state_uninitialized = 0;
        static const uint8_t       _state_destroyed = 1;
        static const uint8_t       _state_initialized = 2;
        static thread_local int32_t _last_error;

        void        abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void        abort_if_not_initialized(const char *method_name) const noexcept;
        static int32_t set_last_operation_error(int32_t error_code) noexcept;


        int32_t     enable_thread_safety(void) noexcept;
        int32_t     disable_thread_safety(void) noexcept;
        bool        is_thread_safe(void) const noexcept;

        int32_t     resize(ft_size_t new_capacity) noexcept;
        static void sleep_backoff() noexcept;
        int32_t     resize_buffer(ft_size_t new_capacity) noexcept;
        int32_t     append_char_buffer(char character) noexcept;
        int32_t     append_buffer(const char *string, ft_size_t length) noexcept;
        int32_t     clear_buffer() noexcept;
        int32_t     ensure_empty_buffer() noexcept;
        int32_t     assign_buffer(ft_size_t count, char character) noexcept;
        int32_t     assign_buffer(const char *string, ft_size_t length) noexcept;
        int32_t     erase_buffer(ft_size_t index, ft_size_t count) noexcept;
        int32_t     resize_length_buffer(ft_size_t new_length) noexcept;
        int32_t     move_buffer(ft_string &other) noexcept;
    public:
        ft_string() noexcept;
        ft_string(const ft_string &other) noexcept;
        ft_string(ft_string &&other) noexcept;
        explicit ft_string(int32_t error_code) noexcept;
        ~ft_string();

        ft_string &operator=(const ft_string &other) noexcept;
        ft_string &operator=(ft_string &&other) noexcept;
        ft_string &operator=(const char *string) noexcept;
        ft_string &operator=(char character) noexcept;

        ft_string &operator+=(const ft_string &string) noexcept;
        ft_string &operator+=(const char *string) noexcept;
        ft_string &operator+=(char character) noexcept;

        bool        operator==(const ft_string &other) const noexcept;
        bool        operator==(const char *string) const noexcept;
        bool        operator!=(const ft_string &other) const noexcept;
        bool        operator!=(const char *string) const noexcept;
        char        operator[](ft_size_t index) const noexcept;
        operator const char*() const noexcept;

        int32_t initialize() noexcept;
        int32_t initialize(const char *initial_string) noexcept;
        int32_t initialize(ft_size_t count, char character) noexcept;
        int32_t initialize(const ft_string &other) noexcept;
        int32_t initialize(ft_string &&other) noexcept;
        int32_t destroy() noexcept;

        int32_t      append(char character) noexcept;
        int32_t      append(const char *string) noexcept;
        int32_t      append(const char *string, ft_size_t length) noexcept;
        int32_t      append(const ft_string &string) noexcept;
        int32_t      assign(ft_size_t count, char character) noexcept;
        int32_t      clear() noexcept;
        int32_t      assign(const char *string, ft_size_t length) noexcept;
        const char    *at(ft_size_t index) const noexcept;
        const char    *c_str() const noexcept;
        char        *data() noexcept;
        const char  *data() const noexcept;
        char*       print() noexcept;
        ft_size_t   size() const noexcept;
        bool        empty() const noexcept;
        int32_t     move(ft_string& other) noexcept;
        int32_t     erase(ft_size_t index, ft_size_t count) noexcept;
        int32_t     push_back(char character) noexcept;
        char        back() noexcept;
        ft_size_t   find(const ft_string &substring) const noexcept;
        ft_size_t   find(const char *substring) const noexcept;
        int32_t     resize_length(ft_size_t new_length) noexcept;
        ft_string   substr(ft_size_t index, ft_size_t count = npos) const noexcept;

        static const ft_size_t npos = static_cast<ft_size_t>(-1);
        static int32_t     last_operation_error() noexcept;
        static const char  *last_operation_error_str() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_string_proxy
{
    private:
        ft_string   _value;
        int32_t     _last_error;

    public:
        ft_string_proxy() noexcept;
        explicit ft_string_proxy(int32_t error_code) noexcept;
        ft_string_proxy(const ft_string &value, int32_t error_code) noexcept;
        ft_string_proxy(const ft_string_proxy &other) noexcept;
        ft_string_proxy(ft_string_proxy &&other) noexcept;
        ~ft_string_proxy();

        ft_string_proxy &operator=(const ft_string_proxy &other) noexcept;
        ft_string_proxy &operator=(ft_string_proxy &&other) noexcept;

        ft_string_proxy operator+(const ft_string &right) const noexcept;
        ft_string_proxy operator+(const char *right) const noexcept;
        ft_string_proxy operator+(char right) const noexcept;

        operator ft_string() const noexcept;
        int32_t get_error() const noexcept;
};

ft_string_proxy operator+(const ft_string &left, const ft_string &right) noexcept;
ft_string_proxy operator+(const ft_string &left, const char *right) noexcept;
ft_string_proxy operator+(const char *left, const ft_string &right) noexcept;
ft_string_proxy operator+(const ft_string &left, char right) noexcept;
ft_string_proxy operator+(char left, const ft_string &right) noexcept;
ft_string_proxy operator+(const ft_string_proxy &left, const ft_string &right) noexcept;
ft_string_proxy operator+(const ft_string_proxy &left, const char *right) noexcept;
ft_string_proxy operator+(const ft_string_proxy &left, char right) noexcept;
bool        operator==(const char *left, const ft_string &right) noexcept;
bool        operator!=(const char *left, const ft_string &right) noexcept;

#endif
