#ifndef FT_FD_ISTREAM_HPP
#define FT_FD_ISTREAM_HPP

#include "class_istream.hpp"
#include "../System_utils/system_utils.hpp"

class ft_fd_istream : public ft_istream
{
    private:
        int _fd;
        mutable pt_recursive_mutex _mutex;
        int lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
    public:
        ft_fd_istream(int fd) noexcept;
        ft_fd_istream(const ft_fd_istream &other) noexcept;
        ft_fd_istream(ft_fd_istream &&other) noexcept;
        ~ft_fd_istream() noexcept;

        ft_fd_istream &operator=(const ft_fd_istream &other) noexcept;
        ft_fd_istream &operator=(ft_fd_istream &&other) noexcept;

        void set_fd(int fd) noexcept;
        int get_fd() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif

    protected:
        std::size_t do_read(char *buffer, std::size_t count);
};

#endif
