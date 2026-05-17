#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "file_utils.hpp"
#include <new>

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

ft_string *file_path_join(const char *path_left, const char *path_right)
{
    ft_string *left;
    ft_string *right;
    ft_string *result;

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
    char path_sep = cmp_path_separator();
    const char *right_data = right->c_str();
    if (right->size() != 0)
    {
        if (right_data[0] == path_sep)
        {
            file_path_delete_string(left);
            return (right);
        }
        if (right->size() >= 2)
        {
            char drive_letter = right_data[0];
            if (((drive_letter >= 'A' && drive_letter <= 'Z') || (drive_letter >= 'a' && drive_letter <= 'z')) && right_data[1] == ':')
            {
                file_path_delete_string(left);
                return (right);
            }
        }
    }
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
        if (data[result->size() - 1] != path_sep)
            result->append(path_sep);
        if (file_string_error(*result) != FT_ERR_SUCCESS)
        {
            file_path_delete_string(result);
            file_path_delete_string(left);
            file_path_delete_string(right);
            return (ft_nullptr);
        }
    }
    ft_size_t index = 0;
    while (right_data[index] == path_sep)
        index++;
    while (right_data[index] != '\0')
    {
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
    file_path_delete_string(left);
    file_path_delete_string(right);
    return (result);
}
