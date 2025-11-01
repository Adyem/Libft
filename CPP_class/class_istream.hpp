#ifndef FT_ISTREAM_HPP
#define FT_ISTREAM_HPP

#include <cstddef>
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_istream
{
    private:
        std::size_t _gcount;
        bool _bad;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno, bool restore_previous_on_success = true) noexcept;

    protected:
        ft_istream() noexcept;
        void set_error(int error_code) const noexcept;
        virtual std::size_t do_read(char *buffer, std::size_t count) = 0;

    public:
        virtual ~ft_istream() noexcept;

        void read(char *buffer, std::size_t count);
        std::size_t gcount() const noexcept;
        bool bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
