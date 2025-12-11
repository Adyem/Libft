#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

    public:
        ft_stringbuf(const ft_string &string) noexcept;
        ft_stringbuf(const ft_stringbuf &other) noexcept;
        ft_stringbuf &operator=(const ft_stringbuf &other) noexcept;
        ft_stringbuf(ft_stringbuf &&other) noexcept;
        ft_stringbuf &operator=(ft_stringbuf &&other) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        ft_string str() const;
};

#endif
