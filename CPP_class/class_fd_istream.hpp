#ifndef FT_FD_ISTREAM_HPP
#define FT_FD_ISTREAM_HPP

#include "class_istream.hpp"
#include "../System_utils/system_utils.hpp"

class ft_fd_istream : public ft_istream
{
    private:
        int _fd;

    public:
        ft_fd_istream(int fd) noexcept;
        ~ft_fd_istream() noexcept;

    protected:
        std::size_t do_read(char *buffer, std::size_t count);
};

#endif
