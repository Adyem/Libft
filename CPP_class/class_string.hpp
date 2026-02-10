#ifndef FT_STRING_HPP
#define FT_STRING_HPP

#include <cstddef>
#include <cstring>
#include <climits>
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/basic.hpp"

class ft_string
{
    private:
        char*                    _data;
        ft_size_t                _length;
        ft_size_t                _capacity;
        mutable pt_recursive_mutex *_mutex;
        mutable bool               _initialized;


	int32_t     enable_thread_safety(void) noexcept;
	int32_t     disable_thread_safety(void) noexcept;
	bool    is_thread_safe(void) const noexcept;

	int32_t resize(ft_size_t new_capacity) noexcept;
	static void sleep_backoff() noexcept;
	int32_t resize_buffer(ft_size_t new_capacity) noexcept;
	int32_t append_char_buffer(char character) noexcept;
	int32_t append_buffer(const char *string, ft_size_t length) noexcept;
	int32_t clear_buffer() noexcept;
	int32_t ensure_empty_buffer() noexcept;
	int32_t assign_buffer(ft_size_t count, char character) noexcept;
	int32_t assign_buffer(const char *string, ft_size_t length) noexcept;
	int32_t erase_buffer(ft_size_t index, ft_size_t count) noexcept;
	int32_t resize_length_buffer(ft_size_t new_length) noexcept;
	int32_t move_buffer(ft_string &other) noexcept;
public:
        ft_string() noexcept;
        ~ft_string();

        ft_string(const ft_string &) = delete;
        ft_string(ft_string &&) = delete;
        ft_string &operator=(const ft_string &) = delete;
        ft_string &operator=(ft_string &&) = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const char *initial_string) noexcept;
        int32_t initialize(ft_size_t count, char character) noexcept;
        int32_t initialize(const ft_string &other) noexcept;
        int32_t initialize(ft_string &&other) noexcept;
        int32_t destroy() noexcept;

        int32_t      append(char character) noexcept;
        int32_t      append(const char *string) noexcept;
        int32_t      append(const char *string, ft_size_t length) noexcept;
        int32_t      append(const ft_string &string) noexcept;
        int32_t      assign(ft_size_t count, char character) noexcept;
        int32_t      clear() noexcept;
        int32_t      assign(const char *string, ft_size_t length) noexcept;
        const char    *at(ft_size_t index) const noexcept;
        const char    *c_str() const noexcept;
        char        *data() noexcept;
        const char  *data() const noexcept;
        char*       print() noexcept;
        ft_size_t   size() const noexcept;
        bool        empty() const noexcept;
        int32_t     move(ft_string& other) noexcept;
        int32_t     erase(ft_size_t index, ft_size_t count) noexcept;
        int32_t     push_back(char character) noexcept;
        char        back() noexcept;
        ft_size_t   find(const ft_string &substring) const noexcept;
        ft_size_t   find(const char *substring) const noexcept;
        int32_t     resize_length(ft_size_t new_length) noexcept;
        ft_string   substr(ft_size_t index, ft_size_t count = npos) const noexcept;

        static const ft_size_t npos = static_cast<ft_size_t>(-1);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
