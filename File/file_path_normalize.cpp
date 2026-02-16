#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "file_utils.hpp"

static int file_string_error(void) noexcept
{
    return (ft_string::last_operation_error());
}

ft_string file_path_normalize(const char *path)
{
    ft_string empty_result;

    if (path == ft_nullptr)
        return (empty_result);
    ft_string original(path);
    if (file_string_error() != FT_ERR_SUCCESS)
        return (original);
    char *data = original.print();
    if (data == ft_nullptr)
        return (empty_result);
    cmp_normalize_slashes(data);
    ft_string result;
    if (file_string_error() != FT_ERR_SUCCESS)
        return (result);
    size_t index = 0;
    char path_sep = cmp_path_separator();
    while (data[index] != '\0')
    {
        if (data[index] == path_sep)
        {
            result.append(path_sep);
            while (data[index] == path_sep)
                index++;
        }
        else
        {
            result.append(data[index]);
            index++;
        }
    }
    if (file_string_error() != FT_ERR_SUCCESS)
        return (result);
    return (result);
}
