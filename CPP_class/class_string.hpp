#ifndef FT_STRING_HPP
#define FT_STRING_HPP

#include <cstddef>
#include <cstring>
#include <climits>
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_string
{
    private:
        char*                    _data;
        std::size_t              _length;
        std::size_t              _capacity;
        mutable pt_recursive_mutex *_mutex;
        mutable bool               _initialized;


        int     enable_thread_safety(void) noexcept;
        int     disable_thread_safety(void) noexcept;
        bool    is_thread_safe_enabled(void) const noexcept;

        int     resize(size_t new_capacity) noexcept;
        static void sleep_backoff() noexcept;
        int     resize_buffer(size_t new_capacity) noexcept;
        int     append_char_buffer(char character) noexcept;
        int     append_buffer(const char *string, size_t length) noexcept;
        int     clear_buffer() noexcept;
        int     ensure_empty_buffer() noexcept;
        int     assign_buffer(size_t count, char character) noexcept;
        int     assign_buffer(const char *string, size_t length) noexcept;
        int     erase_buffer(std::size_t index, std::size_t count) noexcept;
        int     resize_length_buffer(size_t new_length) noexcept;
        int     move_buffer(ft_string &other) noexcept;
public:
        ft_string() noexcept;
        ~ft_string();

        ft_string(const ft_string &) = delete;
        ft_string(ft_string &&) = delete;
        ft_string &operator=(const ft_string &) = delete;
        ft_string &operator=(ft_string &&) = delete;

        int initialize() noexcept;
        int initialize(const char *initial_string) noexcept;
        int initialize(size_t count, char character) noexcept;
        int initialize(const ft_string &other) noexcept;
        int initialize(ft_string &&other) noexcept;
        int destroy() noexcept;

        int         append(char character) noexcept;
        int         append(const char *string) noexcept;
        int         append(const char *string, size_t length) noexcept;
        int         append(const ft_string &string) noexcept;
        int         assign(size_t count, char character) noexcept;
        int         clear() noexcept;
        int         assign(const char *string, size_t length) noexcept;
        const char    *at(size_t index) const noexcept;
        const char    *c_str() const noexcept;
        char        *data() noexcept;
        const char  *data() const noexcept;
        char*       print() noexcept;
        size_t      size() const noexcept;
        bool        empty() const noexcept;
        int         move(ft_string& other) noexcept;
        int         erase(std::size_t index, std::size_t count) noexcept;
        int         push_back(char character) noexcept;
        char        back() noexcept;
        size_t      find(const ft_string &substring) const noexcept;
        size_t      find(const char *substring) const noexcept;
        int         resize_length(size_t new_length) noexcept;
        ft_string   substr(size_t index, size_t count = npos) const noexcept;

        static const size_t npos = static_cast<size_t>(-1);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
