#ifndef FT_BIG_NUMBER_HPP
#define FT_BIG_NUMBER_HPP

#include "../Libft/libft.hpp"

class ft_big_number
{
    private:
        char*           _digits;
        ft_size_t       _size;
        ft_size_t       _capacity;
        mutable int     _error_code;

        void    reserve(ft_size_t new_capacity) noexcept;
        void    shrink_capacity() noexcept;
        void    set_error(int error_code) noexcept;
        bool    is_zero_value() const noexcept;
        int     compare_magnitude(const ft_big_number& other) const noexcept;

    public:
        ft_big_number() noexcept;
        ft_big_number(const ft_big_number& other) noexcept;
        ft_big_number(ft_big_number&& other) noexcept;
        ~ft_big_number();

        ft_big_number& operator=(const ft_big_number& other) noexcept;
        ft_big_number& operator=(ft_big_number&& other) noexcept;
        ft_big_number operator+(const ft_big_number& other) const noexcept;
        ft_big_number operator-(const ft_big_number& other) const noexcept;
        ft_big_number operator*(const ft_big_number& other) const noexcept;
        ft_big_number operator/(const ft_big_number& other) const noexcept;
        bool          operator==(const ft_big_number& other) const noexcept;
        bool          operator!=(const ft_big_number& other) const noexcept;
        bool          operator<(const ft_big_number& other) const noexcept;
        bool          operator<=(const ft_big_number& other) const noexcept;
        bool          operator>(const ft_big_number& other) const noexcept;
        bool          operator>=(const ft_big_number& other) const noexcept;

        void        assign(const char* number) noexcept;
        void        append_digit(char digit) noexcept;
        void        append(const char* digits) noexcept;
        void        append_unsigned(unsigned long value) noexcept;
        void        trim_leading_zeros() noexcept;
        void        reduce_to(ft_size_t new_size) noexcept;
        void        clear() noexcept;

        const char* c_str() const noexcept;
        ft_size_t   size() const noexcept;
        bool        empty() const noexcept;
        int         get_error() const noexcept;
        const char* get_error_str() const noexcept;
};

#endif
