#ifndef FT_STRING_HPP
#define FT_STRING_HPP

#include <cstddef>
#include <cstring>
#include <climits>

class ft_string
{
    private:
        char*            _data;
        std::size_t      _length;
        std::size_t      _capacity;
        mutable int       _error_code;

        void    resize(size_t new_capacity) noexcept;
        void    set_error(int error_code) noexcept;

    public:
        ft_string() noexcept;
        ft_string(const char *init_str) noexcept;
        ft_string(size_t count, char character) noexcept;
        ft_string(const ft_string& other) noexcept;
        ft_string(ft_string&& other) noexcept;
        ft_string &operator=(const ft_string& other) noexcept;
        ft_string &operator=(ft_string&& other) noexcept;
        ft_string &operator=(const char *other) noexcept;
        ft_string& operator+=(const ft_string& other) noexcept;
        ft_string& operator+=(const char* cstr) noexcept;
        ft_string& operator+=(char c) noexcept;
        ~ft_string();

        explicit ft_string(int error_code) noexcept;

        static void* operator new(size_t size) noexcept;
        static void operator delete(void* ptr) noexcept;
        static void* operator new[](size_t size) noexcept;
        static void operator delete[](void* ptr) noexcept;

        void        append(char c) noexcept;
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
        int         get_error() const noexcept;
        const char    *get_error_str() const noexcept;
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

        friend ft_string operator+(const ft_string &lhs, const ft_string &rhs) noexcept;
        friend ft_string operator+(const ft_string &lhs, const char *rhs) noexcept;
        friend ft_string operator+(const char *lhs, const ft_string &rhs) noexcept;
        friend ft_string operator+(const ft_string &lhs, char rhs) noexcept;
        friend ft_string operator+(char lhs, const ft_string &rhs) noexcept;
};

bool operator==(const ft_string &lhs, const ft_string &rhs) noexcept;
bool operator==(const ft_string &lhs, const char* rhs) noexcept;
bool operator==(const char* lhs, const ft_string &rhs) noexcept;

bool operator<(const ft_string &lhs, const ft_string &rhs) noexcept;
bool operator<(const ft_string &lhs, const char* rhs) noexcept;
bool operator<(const char* lhs, const ft_string &rhs) noexcept;

bool operator>(const ft_string &lhs, const ft_string &rhs) noexcept;
bool operator>(const ft_string &lhs, const char* rhs) noexcept;
bool operator>(const char* lhs, const ft_string &rhs) noexcept;

#endif
