#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstring>
#include <climits>
#include <string>

int test_strlen_size_t_null(void)
{
    return (ft_strlen_size_t(ft_nullptr) == 0);
}

int test_strlen_size_t_basic(void)
{
    return (ft_strlen_size_t("abcd") == 4);
}

int test_strlen_size_t_empty(void)
{
    return (ft_strlen_size_t("") == 0);
}

int test_strlen_size_t_long(void)
{
    static char buf[1025];
    for (int i = 0; i < 1024; ++i)
        buf[i] = 'b';
    buf[1024] = '\0';
    return (ft_strlen_size_t(buf) == 1024);
}

int test_bzero_basic(void)
{
    char buf[5] = "abcd";
    ft_bzero(buf, 4);
    for (int i = 0; i < 4; ++i)
    {
        if (buf[i] != '\0')
            return 0;
    }
    return 1;
}

int test_bzero_zero(void)
{
    char buf[5] = "abcd";
    ft_bzero(buf, 0);
    return (ft_strcmp(buf, "abcd") == 0);
}

int test_memcpy_basic(void)
{
    char src[6] = "hello";
    char dst[6];
    ft_memcpy(dst, src, 6);
    return (std::strcmp(dst, src) == 0);
}

int test_memcpy_partial(void)
{
    char src[6] = "world";
    char dst[6] = "aaaaa";
    ft_memcpy(dst, src, 3);
    return (dst[0] == 'w' && dst[1] == 'o' && dst[2] == 'r' && dst[3] == 'a');
}

int test_memcpy_large(void)
{
    char src[1024];
    char dst[1024];
    for (int i = 0; i < 1024; ++i)
        src[i] = static_cast<char>(i % 256);
    ft_memcpy(dst, src, sizeof(src));
    return (std::memcmp(dst, src, sizeof(src)) == 0);
}

int test_memcpy_null(void)
{
    return (ft_memcpy(ft_nullptr, ft_nullptr, 3) == ft_nullptr);
}

int test_memmove_overlap(void)
{
    char str[5] = "abcd";
    ft_memmove(str + 1, str, 3);
    return (std::strcmp(str, "aabc") == 0);
}

int test_memmove_no_overlap(void)
{
    char src[6] = "hello";
    char dst[6];
    ft_memmove(dst, src, 6);
    return (std::strcmp(dst, src) == 0);
}

int test_memchr_found(void)
{
    const char *str = "hello";
    return (ft_memchr(str, 'e', 5) == str + 1);
}

int test_memchr_not_found(void)
{
    const char *str = "hello";
    return (ft_memchr(str, 'x', 5) == ft_nullptr);
}

int test_memcmp_basic(void)
{
    char a[3] = {'a','b','c'};
    char b[3] = {'a','b','c'};
    return (ft_memcmp(a, b, 3) == 0);
}

int test_memcmp_diff(void)
{
    char a[3] = {'a','b','d'};
    char b[3] = {'a','b','c'};
    return (ft_memcmp(a, b, 3) > 0);
}

int test_strchr_basic(void)
{
    const char *str = "hello";
    return (ft_strchr(str, 'e') == str + 1);
}

int test_strchr_not_found(void)
{
    const char *str = "hello";
    return (ft_strchr(str, 'x') == ft_nullptr);
}

int test_strrchr_basic(void)
{
    const char *str = "hello";
    return (ft_strrchr(str, 'l') == str + 3);
}

int test_strrchr_not_found(void)
{
    const char *str = "hello";
    return (ft_strrchr(str, 'z') == ft_nullptr);
}

int test_strnstr_basic(void)
{
    const char *str = "hello world";
    const char *found = ft_strnstr(str, "world", 11);
    return (found == str + 6);
}

int test_strnstr_not_found(void)
{
    const char *str = "hello world";
    return (ft_strnstr(str, "42", 11) == ft_nullptr);
}

int test_strlcpy_basic(void)
{
    char dst[6];
    size_t r = ft_strlcpy(dst, "hello", sizeof(dst));
    return (r == 5 && std::strcmp(dst, "hello") == 0);
}

int test_strlcpy_truncate(void)
{
    char dst[3];
    size_t r = ft_strlcpy(dst, "hello", sizeof(dst));
    return (r == 5 && std::strcmp(dst, "he") == 0);
}

int test_strlcat_basic(void)
{
    char dst[10] = "hi";
    size_t r = ft_strlcat(dst, "1234", sizeof(dst));
    return (r == 6 && std::strcmp(dst, "hi1234") == 0);
}

int test_strlcat_truncate(void)
{
    char dst[6] = "hi";
    size_t r = ft_strlcat(dst, "world", sizeof(dst));
    return (r == 7 && std::strcmp(dst, "hiwor") == 0);
}

int test_strncmp_basic(void)
{
    return (ft_strncmp("abc", "abd", 2) == 0);
}

int test_strncmp_diff(void)
{
    return (ft_strncmp("abc", "abd", 3) < 0);
}

int test_strncpy_basic(void)
{
    char dst[5];
    ft_strncpy(dst, "abc", 4);
    return (std::strcmp(dst, "abc") == 0);
}

int test_strncpy_padding(void)
{
    char dst[6];
    ft_strncpy(dst, "abc", 5);
    return (dst[3] == '\0' && dst[4] == '\0');
}

int test_isalpha_true(void)
{
    return (ft_isalpha('A') == 1);
}

int test_isalpha_lower_true(void)
{
    return (ft_isalpha('z') == 1);
}

int test_isalpha_false(void)
{
    return (ft_isalpha('1') == 0);
}

int test_isalnum_true(void)
{
    return (ft_isalnum('1') == 1 && ft_isalnum('a') == 1);
}

int test_isalnum_false(void)
{
    return (ft_isalnum('#') == 0);
}

int test_isspace_true(void)
{
    return (ft_isspace(' ') == 1);
}

int test_isspace_false(void)
{
    return (ft_isspace('a') == 0);
}

int test_tolower_basic(void)
{
    char str[4] = "ABC";
    ft_to_lower(str);
    return (std::strcmp(str, "abc") == 0);
}

int test_tolower_no_change(void)
{
    char str[4] = "abc";
    ft_to_lower(str);
    return (std::strcmp(str, "abc") == 0);
}

int test_abs_basic(void)
{
    return (ft_abs(-5) == 5);
}

int test_abs_zero(void)
{
    return (ft_abs(0) == 0);
}

int test_abs_positive(void)
{
    return (ft_abs(5) == 5);
}

int test_atol_basic(void)
{
    return (ft_atol("-42") == -42);
}

int test_atol_whitespace(void)
{
    return (ft_atol("  42") == 42);
}

int test_atol_longmax(void)
{
    std::string s = std::to_string(LONG_MAX);
    return (ft_atol(s.c_str()) == LONG_MAX);
}

int test_atol_longmin(void)
{
    std::string s = std::to_string(LONG_MIN);
    return (ft_atol(s.c_str()) == LONG_MIN);
}

