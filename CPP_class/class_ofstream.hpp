#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"

class ft_ofstream
{
    private:
        ft_file _file;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        ft_ofstream() noexcept;
        ~ft_ofstream() noexcept;

        int open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        void close() noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
