#include "compatebility_internal.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if defined(_WIN32) || defined(_WIN64)
char cmp_path_separator(void)
{
    return ('\\');
}

void cmp_normalize_slashes(char *data)
{
    if (!data)
        return ;
    ft_size_t index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '/')
            data[index] = '\\';
        index++;
    }
    return ;
}
#else
char cmp_path_separator(void)
{
    return ('/');
}

void cmp_normalize_slashes(char *data)
{
    if (!data)
        return ;
    ft_size_t index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '\\')
            data[index] = '/';
        index++;
    }
    return ;
}
#endif

#if defined(_WIN32) || defined(_WIN64)
static char cmp_path_lower_ascii(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character + ('a' - 'A')));
    return (character);
}
#endif

ft_bool cmp_path_equal(const char *path_left, const char *path_right) noexcept
{
    ft_size_t index;

    if (path_left == ft_nullptr || path_right == ft_nullptr)
        return (FT_FALSE);
    index = 0;
    while (path_left[index] != '\0' && path_right[index] != '\0')
    {
#if defined(_WIN32) || defined(_WIN64)
        if (cmp_path_lower_ascii(path_left[index])
            != cmp_path_lower_ascii(path_right[index]))
            return (FT_FALSE);
#else
        if (path_left[index] != path_right[index])
            return (FT_FALSE);
#endif
        ++index;
    }
    if (path_left[index] != path_right[index])
        return (FT_FALSE);
    return (FT_TRUE);
}

