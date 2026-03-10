#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class ft_ofstream
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        ft_file _file;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;

    public:
        ft_ofstream() noexcept;
        ft_ofstream(const ft_ofstream &other) noexcept;
        ft_ofstream(ft_ofstream &&other) noexcept;
        ~ft_ofstream() noexcept;

        ft_ofstream &operator=(const ft_ofstream &other) noexcept = delete;
        ft_ofstream &operator=(ft_ofstream &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t destroy() noexcept;
        int32_t move(ft_ofstream &other) noexcept;
        int32_t enable_thread_safety(void) noexcept;
        int32_t disable_thread_safety(void) noexcept;
        ft_bool is_thread_safe(void) const noexcept;

        int32_t open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        int32_t close() noexcept;
};

#endif
