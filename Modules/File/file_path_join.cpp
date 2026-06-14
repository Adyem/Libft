#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "file_utils.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t file_string_error(const ft_string &string_value) noexcept
{
    return (string_value.get_error());
}

static void file_path_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

static ft_bool file_path_is_drive_letter(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (FT_TRUE);
    if (character >= 'a' && character <= 'z')
        return (FT_TRUE);
    return (FT_FALSE);
}

static void file_path_convert_separator(ft_string &path_value,
    char source_separator, char destination_separator) noexcept
{
    char *data;
    ft_size_t index;

    if (source_separator == destination_separator)
        return ;
    data = path_value.print();
    if (data == ft_nullptr)
        return ;
    index = 0;
    while (index < path_value.size())
    {
        if (data[index] == source_separator)
            data[index] = destination_separator;
        index++;
    }
    return ;
}

static void file_path_normalize_to_forward_slashes(ft_string &path_value) noexcept
{
    file_path_convert_separator(path_value, '\\', '/');
    return ;
}

ft_string *file_path_join(const char *path_left, const char *path_right)
{
    ft_string *left;
    ft_string *right;
    ft_string *result;
    const char *right_data;
    char output_separator;
    ft_bool left_is_absolute;
    ft_bool right_is_absolute;
    ft_size_t index;

    left = file_path_normalize(path_left);
    if (left == ft_nullptr)
        return (ft_nullptr);
    if (file_string_error(*left) != FT_ERR_SUCCESS)
    {
        file_path_delete_string(left);
        return (ft_nullptr);
    }
    right = file_path_normalize(path_right);
    if (right == ft_nullptr)
    {
        file_path_delete_string(left);
        return (ft_nullptr);
    }
    if (file_string_error(*right) != FT_ERR_SUCCESS)
    {
        file_path_delete_string(left);
        file_path_delete_string(right);
        return (ft_nullptr);
    }
    right_data = right->c_str();
    right_is_absolute = FT_FALSE;
    if (right->size() > 0)
    {
        if (right_data[0] == '/' || right_data[0] == '\\')
            right_is_absolute = FT_TRUE;
        else if (right->size() >= 2
            && file_path_is_drive_letter(right_data[0]) == FT_TRUE
            && right_data[1] == ':')
            right_is_absolute = FT_TRUE;
    }
    if (right_is_absolute == FT_TRUE)
    {
        file_path_normalize_to_forward_slashes(*right);
        file_path_delete_string(left);
        return (right);
    }
    output_separator = '/';
    left_is_absolute = FT_FALSE;
    if (left->size() > 0)
    {
        if (left->c_str()[0] == '/' || left->c_str()[0] == '\\')
            left_is_absolute = FT_TRUE;
        else if (left->size() >= 2
            && file_path_is_drive_letter(left->c_str()[0]) == FT_TRUE
            && left->c_str()[1] == ':')
            left_is_absolute = FT_TRUE;
    }
    if (left->size() == 0 || left_is_absolute == FT_TRUE)
        output_separator = cmp_path_separator();
    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
    {
        file_path_delete_string(left);
        file_path_delete_string(right);
        return (ft_nullptr);
    }
    if (result->initialize(*left) != FT_ERR_SUCCESS)
    {
        delete result;
        file_path_delete_string(left);
        file_path_delete_string(right);
        return (ft_nullptr);
    }
    if (file_string_error(*result) != FT_ERR_SUCCESS)
    {
        file_path_delete_string(result);
        file_path_delete_string(left);
        file_path_delete_string(right);
        return (ft_nullptr);
    }
    if (result->size() != 0)
    {
        const char *data = result->c_str();

        if (data[result->size() - 1] != output_separator)
            result->append(output_separator);
        if (file_string_error(*result) != FT_ERR_SUCCESS)
        {
            file_path_delete_string(result);
            file_path_delete_string(left);
            file_path_delete_string(right);
            return (ft_nullptr);
        }
    }
    index = 0;
    while (right_data[index] == '/' || right_data[index] == '\\')
        index++;
    while (right_data[index] != '\0')
    {
        if (right_data[index] == '/' || right_data[index] == '\\')
            result->append(output_separator);
        else
            result->append(right_data[index]);
        if (file_string_error(*result) != FT_ERR_SUCCESS)
        {
            file_path_delete_string(result);
            file_path_delete_string(left);
            file_path_delete_string(right);
            return (ft_nullptr);
        }
        index++;
    }
    if (left->size() == 0)
        file_path_convert_separator(*result, '/', cmp_path_separator());
    file_path_delete_string(left);
    file_path_delete_string(right);
    return (result);
}
