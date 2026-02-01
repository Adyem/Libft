#ifndef FT_STRING_HPP
#define FT_STRING_HPP

#include <cstddef>
#include <cstring>
#include <climits>
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_string
{
    private:
        char*            _data;
        std::size_t      _length;
        std::size_t      _capacity;
        mutable pt_recursive_mutex  _mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        friend ft_operation_error_stack *ft_string_error_stack_owner(void) noexcept;

        class mutex_guard
        {
            private:
                pt_recursive_mutex   *_mutex;
                bool        _owns_lock;
                int         _error_code;

            public:
                mutex_guard() noexcept;
                ~mutex_guard();

                mutex_guard(const mutex_guard &other) = delete;
                mutex_guard &operator=(const mutex_guard &other) = delete;
                mutex_guard(mutex_guard &&other) noexcept;
                mutex_guard &operator=(mutex_guard &&other) noexcept;

                int lock(pt_recursive_mutex &mutex) noexcept;
                void unlock() noexcept;
                bool owns_lock() const noexcept;
                int get_error() const noexcept;
        };

        void    resize(size_t new_capacity) noexcept;
        unsigned long long    push_error_unlocked(int error_code) const noexcept;
        unsigned long long    push_error(int error_code) const noexcept;
        int     lock_self(mutex_guard &guard) const noexcept;
        static int  lock_pair(const ft_string &first, const ft_string &second,
                mutex_guard &first_guard,
                mutex_guard &second_guard) noexcept;
        static void sleep_backoff() noexcept;
        void    resize_unlocked(size_t new_capacity) noexcept;
        void    append_char_unlocked(char character) noexcept;
        void    append_unlocked(const char *string, size_t length) noexcept;
        void    clear_unlocked() noexcept;
        void    ensure_empty_buffer_unlocked() noexcept;
        void    assign_unlocked(size_t count, char character) noexcept;
        void    assign_unlocked(const char *string, size_t length) noexcept;
        void    erase_unlocked(std::size_t index, std::size_t count) noexcept;
        void    resize_length_unlocked(size_t new_length) noexcept;
        void    move_unlocked(ft_string &other) noexcept;
        static void reset_error_owner(const ft_string *owner) noexcept;

    public:
        ft_string() noexcept;
        ft_string(const char *initial_string) noexcept;
        ft_string(size_t count, char character) noexcept;
        ft_string(const ft_string& other) noexcept;
        ft_string(ft_string&& other) noexcept;
        ft_string &operator=(const ft_string& other) noexcept;
        ft_string &operator=(ft_string&& other) noexcept;
        ft_string &operator=(const char *other) noexcept;
        ft_string& operator+=(const ft_string& other) noexcept;
        ft_string& operator+=(const char* string_value) noexcept;
        ft_string& operator+=(char character) noexcept;
        ~ft_string();

        explicit ft_string(int error_code) noexcept;

        static void* operator new(size_t size) noexcept;
        static void operator delete(void* pointer) noexcept;
        static void* operator new[](size_t size) noexcept;
        static void operator delete[](void* pointer) noexcept;

        void        append(char character) noexcept;
        void        append(const char *string) noexcept;
        void        append(const char *string, size_t length) noexcept;
        void        append(const ft_string &string) noexcept;
        void        assign(size_t count, char character) noexcept;
        void        clear() noexcept;
        void        assign(const char *string, size_t length) noexcept;
        const char    *at(size_t index) const noexcept;
        const char    *c_str() const noexcept;
        char        *data() noexcept;
        const char  *data() const noexcept;
        char*       print() noexcept;
        size_t      size() const noexcept;
        bool        empty() const noexcept;
        static const char *last_operation_error_str() noexcept;
        static const char *operation_error_str_at(size_t index) noexcept;
        static int  last_operation_error() noexcept;
        static int  operation_error_at(size_t index) noexcept;
        static void pop_operation_errors() noexcept;
        static int  pop_oldest_operation_error() noexcept;
        static int  operation_error_index() noexcept;
        unsigned long long last_operation_id() const noexcept;
        int pop_operation_error(unsigned long long operation_id) const noexcept;
        void        move(ft_string& other) noexcept;
        void        erase(std::size_t index, std::size_t count) noexcept;
        void        push_back(char character) noexcept;
        char        back() noexcept;
        size_t      find(const ft_string &substring) const noexcept;
        size_t      find(const char *substring) const noexcept;
        void        resize_length(size_t new_length) noexcept;
        ft_string   substr(size_t index, size_t count = npos) const noexcept;

        static const size_t npos = static_cast<size_t>(-1);

        operator const char*() const noexcept;

        friend ft_string operator+(const ft_string &left_value,
            const ft_string &right_value) noexcept;
        friend ft_string operator+(const ft_string &left_value,
            const char *right_value) noexcept;
        friend ft_string operator+(const char *left_value,
            const ft_string &right_value) noexcept;
        friend ft_string operator+(const ft_string &left_value,
            char right_character) noexcept;
        friend ft_string operator+(char left_character,
            const ft_string &right_value) noexcept;

#ifdef LIBFT_TEST_BUILD
        ft_operation_error_stack *operation_error_stack_handle() const noexcept;
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

bool operator==(const ft_string &left_value, const ft_string &right_value) noexcept;
bool operator==(const ft_string &left_value, const char* right_value) noexcept;
bool operator==(const char* left_value, const ft_string &right_value) noexcept;
bool operator!=(const ft_string &left_value, const ft_string &right_value) noexcept;
bool operator!=(const ft_string &left_value, const char* right_value) noexcept;
bool operator!=(const char* left_value, const ft_string &right_value) noexcept;

bool operator<(const ft_string &left_value, const ft_string &right_value) noexcept;
bool operator<(const ft_string &left_value, const char* right_value) noexcept;
bool operator<(const char* left_value, const ft_string &right_value) noexcept;

bool operator>(const ft_string &left_value, const ft_string &right_value) noexcept;
bool operator>(const ft_string &left_value, const char* right_value) noexcept;
bool operator>(const char* left_value, const ft_string &right_value) noexcept;

#endif
