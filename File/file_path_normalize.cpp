#include "../CPP_class/class_string_class.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "file_utils.hpp"

ft_string file_path_normalize(const char *path)
{
    ft_string original(path);
    if (original.get_error())
        return (original);
    char *data = original.print();
    cmp_normalize_slashes(data);
    ft_string result;
    if (result.get_error())
        return (result);
    size_t index = 0;
    char path_sep = cmp_path_separator();
    while (data[index] != '\0')
    {
        if (data[index] == path_sep)
        {
            result.append(path_sep);
            while (data[index] == path_sep)
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
