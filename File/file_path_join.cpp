#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "file_utils.hpp"

static int file_string_error(void) noexcept
{
    return (ft_string::last_operation_error());
}

ft_string file_path_join(const char *path_left, const char *path_right)
{
    ft_string left = file_path_normalize(path_left);
    if (file_string_error() != FT_ERR_SUCCESS)
        return (left);
    ft_string right = file_path_normalize(path_right);
    if (file_string_error() != FT_ERR_SUCCESS)
        return (right);
    char path_sep = cmp_path_separator();
    const char *right_data = right.c_str();
    if (right.size() != 0)
    {
        if (right_data[0] == path_sep)
            return (right);
        if (right.size() >= 2)
        {
            char drive_letter = right_data[0];
            if (((drive_letter >= 'A' && drive_letter <= 'Z') || (drive_letter >= 'a' && drive_letter <= 'z')) && right_data[1] == ':')
                return (right);
        }
    }
    ft_string result(left);
    if (file_string_error() != FT_ERR_SUCCESS)
        return (result);
    if (result.size() != 0)
    {
        const char *data = result.c_str();
        if (data[result.size() - 1] != path_sep)
            result.append(path_sep);
    }
    size_t index = 0;
    while (right_data[index] == path_sep)
        index++;
    while (right_data[index] != '\0')
    {
        result.append(right_data[index]);
        index++;
    }
    return (result);
}
