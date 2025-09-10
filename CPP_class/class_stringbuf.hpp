#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string_class.hpp"
#include "../Errno/errno.hpp"

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        ft_stringbuf(const ft_string &string) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        ft_string str() const;
};

#endif
