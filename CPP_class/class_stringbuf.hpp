#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
        mutable pt_recursive_mutex _mutex;
        int lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
        static int lock_pair(const ft_stringbuf &first, const ft_stringbuf &second,
            ft_unique_lock<pt_recursive_mutex> &first_guard,
            ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept;

    public:
        ft_stringbuf(const ft_string &string) noexcept;
        ft_stringbuf(const ft_stringbuf &other) noexcept;
        ft_stringbuf &operator=(const ft_stringbuf &other) noexcept;
        ft_stringbuf(ft_stringbuf &&other) noexcept;
        ft_stringbuf &operator=(ft_stringbuf &&other) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_valid() const noexcept;
        ft_string str() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
