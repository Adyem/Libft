#include "compatebility_internal.hpp"

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
