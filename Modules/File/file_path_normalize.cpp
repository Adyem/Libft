#include "../CPP_class/class_nullptr.hpp"
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

ft_string *file_path_normalize(const char *path)
{
    ft_string *result;
    int32_t initialization_error;

    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    initialization_error = result->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }

    if (path == ft_nullptr)
        return (result);
    ft_string original;
    if (original.initialize(path) != FT_ERR_SUCCESS)
    {
        file_path_delete_string(result);
        return (ft_nullptr);
    }
    if (file_string_error(original) != FT_ERR_SUCCESS)
    {
        file_path_delete_string(result);
        return (ft_nullptr);
    }
    char *data = original.print();
    if (data == ft_nullptr)
    {
        file_path_delete_string(result);
        return (ft_nullptr);
    }
    cmp_normalize_slashes(data);
    ft_size_t index = 0;
    char path_sep = cmp_path_separator();
    while (data[index] != '\0')
    {
        if (data[index] == path_sep)
        {
            result->append(path_sep);
            if (file_string_error(*result) != FT_ERR_SUCCESS)
            {
                file_path_delete_string(result);
                return (ft_nullptr);
            }
            while (data[index] == path_sep)
                index++;
        }
        else
        {
            result->append(data[index]);
            if (file_string_error(*result) != FT_ERR_SUCCESS)
            {
                file_path_delete_string(result);
                return (ft_nullptr);
            }
            index++;
        }
    }
    return (result);
}
