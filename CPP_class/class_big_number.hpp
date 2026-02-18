#ifndef FT_BIG_NUMBER_HPP
#define FT_BIG_NUMBER_HPP

#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_big_number_proxy;

class ft_big_number
{
    friend class ft_big_number_proxy;

    private:
        char*           _digits;
        ft_size_t       _size;
        ft_size_t       _capacity;
        bool            _is_negative;
        mutable pt_recursive_mutex *_mutex;
        mutable uint8_t            _initialized_state;
        int             _operation_error;
        static const uint8_t       _state_uninitialized = 0;
        static const uint8_t       _state_destroyed = 1;
        static const uint8_t       _state_initialized = 2;
        static thread_local int _last_error;

		void    abort_lifecycle_error(const char *method_name, const char *reason) const noexcept;
		void    abort_if_not_initialized(const char *method_name) const noexcept;
        void    reserve(ft_size_t new_capacity) noexcept;
        void    shrink_capacity() noexcept;
        static int set_last_operation_error(int error_code) noexcept;
        int     lock_mutex(void) const noexcept;
        int     unlock_mutex(void) const noexcept;
        static int  lock_pair(const ft_big_number &first, const ft_big_number &second,
                const ft_big_number *&lower,
                const ft_big_number *&upper) noexcept;
        static int  unlock_pair(const ft_big_number *lower, const ft_big_number *upper) noexcept;
        static void sleep_backoff() noexcept;
        void    clear_unlocked() noexcept;
        void    append_digit_unlocked(char digit) noexcept;
        void    append_unlocked(const char* digits) noexcept;
        void    append_unsigned_unlocked(unsigned long value) noexcept;
        void    trim_leading_zeros_unlocked() noexcept;
        void    reduce_to_unlocked(ft_size_t new_size) noexcept;
        bool    is_zero_value() const noexcept;
        int     compare_magnitude(const ft_big_number& other) const noexcept;
        ft_big_number    add_magnitude(const ft_big_number& other) const noexcept;
        ft_big_number    subtract_magnitude(const ft_big_number& other) const noexcept;

    public:
        ft_big_number() noexcept;
        ft_big_number(const ft_big_number& other) noexcept;
        ft_big_number(ft_big_number&& other) noexcept;
        ~ft_big_number() noexcept;

        ft_big_number& operator=(const ft_big_number& other) noexcept;
        ft_big_number& operator=(ft_big_number&& other) noexcept;
        ft_big_number_proxy operator+(const ft_big_number& other) const noexcept;
        ft_big_number_proxy operator-(const ft_big_number& other) const noexcept;
        ft_big_number_proxy operator*(const ft_big_number& other) const noexcept;
        ft_big_number_proxy operator/(const ft_big_number& other) const noexcept;
        ft_big_number_proxy operator%(const ft_big_number& other) const noexcept;
        bool          operator==(const ft_big_number& other) const noexcept;
        bool          operator!=(const ft_big_number& other) const noexcept;
        bool          operator<(const ft_big_number& other) const noexcept;
        bool          operator<=(const ft_big_number& other) const noexcept;
        bool          operator>(const ft_big_number& other) const noexcept;
        bool          operator>=(const ft_big_number& other) const noexcept;

        void        assign(const char* number) noexcept;
        void        assign_base(const char* digits, int base) noexcept;
        void        append_digit(char digit) noexcept;
        void        append(const char* digits) noexcept;
        void        append_unsigned(unsigned long value) noexcept;
        void        trim_leading_zeros() noexcept;
        void        reduce_to(ft_size_t new_size) noexcept;
        void        clear() noexcept;

        const char* c_str() const noexcept;
        ft_size_t   size() const noexcept;
        bool        empty() const noexcept;
        bool        is_negative() const noexcept;
        bool        is_positive() const noexcept;
        ft_string   to_string_base(int base) noexcept;
        ft_big_number mod_pow(const ft_big_number& exponent, const ft_big_number& modulus) const noexcept;
        static const char *last_operation_error_str() noexcept;
        int     enable_thread_safety(void) noexcept;
        int     disable_thread_safety(void) noexcept;
        bool    is_thread_safe(void) const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex    *get_mutex_for_testing() noexcept;
#endif
};

class ft_big_number_proxy
{
    private:
        ft_big_number _value;
        int _last_error;

    public:
        ft_big_number_proxy() noexcept;
        explicit ft_big_number_proxy(int error_code) noexcept;
        ft_big_number_proxy(const ft_big_number &value) noexcept;
        ft_big_number_proxy(const ft_big_number &value, int error_code) noexcept;
        ft_big_number_proxy(const ft_big_number_proxy &other) noexcept;
        ft_big_number_proxy(ft_big_number_proxy &&other) noexcept;
        ~ft_big_number_proxy();

        ft_big_number_proxy &operator=(const ft_big_number_proxy &other) noexcept;
        ft_big_number_proxy &operator=(ft_big_number_proxy &&other) noexcept;

        ft_big_number_proxy operator+(const ft_big_number &right) const noexcept;
        ft_big_number_proxy operator-(const ft_big_number &right) const noexcept;
        ft_big_number_proxy operator*(const ft_big_number &right) const noexcept;
        ft_big_number_proxy operator/(const ft_big_number &right) const noexcept;
        ft_big_number_proxy operator%(const ft_big_number &right) const noexcept;

        operator ft_big_number() const noexcept;
        int get_error() const noexcept;
};

ft_big_number_proxy operator+(const ft_big_number_proxy &left, const ft_big_number &right) noexcept;
ft_big_number_proxy operator-(const ft_big_number_proxy &left, const ft_big_number &right) noexcept;
ft_big_number_proxy operator*(const ft_big_number_proxy &left, const ft_big_number &right) noexcept;
ft_big_number_proxy operator/(const ft_big_number_proxy &left, const ft_big_number &right) noexcept;
ft_big_number_proxy operator%(const ft_big_number_proxy &left, const ft_big_number &right) noexcept;

ft_string   big_number_to_hex_string(const ft_big_number& number) noexcept;
ft_big_number   big_number_from_hex_string(const char* hex_digits) noexcept;

#endif
