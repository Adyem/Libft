#include "../CPP_class/class_string_class.hpp"
#include "file_utils.hpp"

#if defined(_WIN32) || defined(_WIN64)
# define PATH_SEP '\\'
#else
# define PATH_SEP '/'
#endif

ft_string file_path_join(const char *path_left, const char *path_right)
{
    ft_string left = file_path_normalize(path_left);
    if (left.get_error())
        return (left);
    ft_string right = file_path_normalize(path_right);
    if (right.get_error())
        return (right);
    ft_string result(left);
    if (result.get_error())
        return (result);
    if (result.size() != 0)
    {
        const char *data = result.c_str();
        if (data[result.size() - 1] != PATH_SEP)
            result.append(PATH_SEP);
    }
    const char *right_data = right.c_str();
    size_t index = 0;
    while (right_data[index] == PATH_SEP)
    {
        ++index;
    }
    while (right_data[index] != '\0')
    {
        result.append(right_data[index]);
        ++index;
    }
    if (result.get_error())
        return (result);
    return (result);
}
