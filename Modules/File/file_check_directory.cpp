#include <cstdlib>
#include "../CPP_class/class_string.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "open_dir.hpp"

static int32_t normalize_path(ft_string &path) noexcept
{
    char *data = path.print();

    if (data == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    cmp_normalize_slashes(data);
    return (FT_ERR_SUCCESS);
}

int32_t file_dir_exists(const char *rel_path)
{
    ft_string path;
    int32_t initialization_error;

    initialization_error = path.initialize(rel_path);
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    int32_t error_code;
    int32_t exists_value;
    int32_t status;

    error_code = normalize_path(path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (path.get_error() != FT_ERR_SUCCESS)
        return (path.get_error());
    exists_value = 0;
    status = cmp_directory_exists(path.c_str(), &exists_value, &error_code);
    if (status != FT_ERR_SUCCESS)
        return (error_code);
    if (exists_value == FT_ERR_SUCCESS)
        return (FT_ERR_NOT_FOUND);
    return (FT_ERR_SUCCESS);
}
