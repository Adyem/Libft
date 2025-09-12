#include "../CPP_class/class_string_class.hpp"
#include "file_utils.hpp"

#if defined(_WIN32) || defined(_WIN64)
# define PATH_SEP '\\'
#else
# define PATH_SEP '/'
#endif

ft_string file_path_normalize(const char *path)
{
    ft_string original(path);
    if (original.get_error())
        return (original);
    char *data = original.print();
    size_t index = 0;
    while (data[index] != '\0')
    {
#if defined(_WIN32) || defined(_WIN64)
        if (data[index] == '/')
            data[index] = '\\';
#else
        if (data[index] == '\\')
            data[index] = '/';
#endif
        ++index;
    }
    ft_string result;
    if (result.get_error())
        return (result);
    index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == PATH_SEP)
        {
            result.append(PATH_SEP);
            while (data[index] == PATH_SEP)
            {
                ++index;
            }
        }
        else
        {
            result.append(data[index]);
            ++index;
        }
    }
    if (result.get_error())
        return (result);
    return (result);
}
