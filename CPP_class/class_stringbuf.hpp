#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include <string>
#include "../Errno/errno.hpp"

class ft_stringbuf
{
    private:
        std::string _storage;
        std::size_t _position;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        ft_stringbuf(const std::string &string) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        std::string str() const;
};

#endif
