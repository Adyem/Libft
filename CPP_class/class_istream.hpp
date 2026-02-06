#ifndef FT_ISTREAM_HPP
#define FT_ISTREAM_HPP

#include <cstddef>
#include "../PThread/recursive_mutex.hpp"

class ft_istream
{
    private:
        std::size_t _gcount;
        bool _is_valid;
        mutable pt_recursive_mutex *_mutex;

        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;
        static int lock_pair(const ft_istream &first, const ft_istream &second,
                const ft_istream *&lower, const ft_istream *&upper) noexcept;
        static int unlock_pair(const ft_istream *lower, const ft_istream *upper) noexcept;

    protected:
        ft_istream() noexcept;
        ft_istream(const ft_istream &other) noexcept;
        ft_istream(ft_istream &&other) noexcept;
        virtual std::size_t do_read(char *buffer, std::size_t count) = 0;

    public:
        virtual ~ft_istream() noexcept;

        ft_istream &operator=(const ft_istream &other) noexcept;
        ft_istream &operator=(ft_istream &&other) noexcept;

        void read(char *buffer, std::size_t count);
        std::size_t gcount() const noexcept;
        bool is_valid() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
