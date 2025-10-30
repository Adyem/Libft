#ifndef FT_FD_ISTREAM_HPP
#define FT_FD_ISTREAM_HPP

#include "class_istream.hpp"
#include "../System_utils/system_utils.hpp"

class ft_fd_istream : public ft_istream
{
    private:
        int _fd;
        mutable pt_mutex _mutex;

        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard,
            int entry_errno) noexcept;

    public:
        ft_fd_istream(int fd) noexcept;
        ~ft_fd_istream() noexcept;

        void set_fd(int fd) noexcept;
        int get_fd() const noexcept;

    protected:
        std::size_t do_read(char *buffer, std::size_t count);
};

#endif
