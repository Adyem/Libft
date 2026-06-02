#include "filesystem.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../File/file_utils.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_string *filesystem_empty_string(void)
{
    ft_string *result;

    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    if (result->initialize() != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}

static ft_string *filesystem_string_from_owned_c_string(char *value)
{
    ft_string *result;

    if (value == ft_nullptr)
        return (filesystem_empty_string());
    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
    {
        cma_free(value);
        return (ft_nullptr);
    }
    if (result->initialize(value) != FT_ERR_SUCCESS)
    {
        cma_free(value);
        delete result;
        return (ft_nullptr);
    }
    cma_free(value);
    return (result);
}

ft_string *filesystem_normalize_path(const char *path)
{
    return (file_path_normalize(path));
}

ft_string *filesystem_join_path(const char *path_left, const char *path_right)
{
    return (file_path_join(path_left, path_right));
}

ft_string *filesystem_basename(const char *path)
{
    return (filesystem_string_from_owned_c_string(file_path_basename(path)));
}

ft_string *filesystem_dirname(const char *path)
{
    return (filesystem_string_from_owned_c_string(file_path_dirname(path)));
}

ft_string *filesystem_extension(const char *path)
{
    return (filesystem_string_from_owned_c_string(file_path_extension(path)));
}

ft_string *filesystem_stem(const char *path)
{
    return (filesystem_string_from_owned_c_string(file_path_stem(path)));
}

ft_bool filesystem_is_absolute(const char *path) noexcept
{
    return (file_path_is_absolute(path));
}

ft_bool filesystem_is_relative(const char *path) noexcept
{
    return (file_path_is_relative(path));
}

static ft_bool filesystem_strings_equal(const char *left, const char *right) noexcept
{
    ft_size_t index;

    if (left == ft_nullptr || right == ft_nullptr)
    {
        return (FT_FALSE);
    }
    index = 0;
    while (left[index] != '\0' && right[index] != '\0')
    {
        if (left[index] != right[index])
        {
            return (FT_FALSE);
        }
        index++;
    }
    if (left[index] == right[index])
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

ft_bool filesystem_has_extension(const char *path, const char *extension) noexcept
{
    char *path_extension;
    ft_bool result;

    if (path == ft_nullptr || extension == ft_nullptr)
    {
        return (FT_FALSE);
    }
    path_extension = file_path_extension(path);
    if (path_extension == ft_nullptr)
    {
        return (FT_FALSE);
    }
    result = filesystem_strings_equal(path_extension, extension);
    cma_free(path_extension);
    return (result);
}

