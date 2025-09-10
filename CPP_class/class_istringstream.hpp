#ifndef FT_ISTRINGSTREAM_HPP
#define FT_ISTRINGSTREAM_HPP

#include <string>
#include <sstream>
#include <istream>
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"

class ft_istringstream : public std::istream
{
    private:
        std::stringbuf _buffer;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        ft_istringstream(const std::string &string) noexcept;
        ~ft_istringstream() noexcept;

        ft_istringstream(const ft_istringstream &) = delete;
        ft_istringstream &operator=(const ft_istringstream &) = delete;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
