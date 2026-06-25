#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_bool filesystem_path_is_parent_reference(const char *path) noexcept
{
    if (path == ft_nullptr)
        return (FT_FALSE);
    if (path[0] == '.' && path[1] == '.' && path[2] == '\0')
        return (FT_TRUE);
    if (path[0] == '.' && path[1] == '.'
        && (path[2] == '/' || path[2] == '\\'))
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool filesystem_is_safe_relative_path(const char *path) noexcept
{
    ft_string *normalized_path;
    ft_bool result;

    if (path == ft_nullptr || path[0] == '\0')
        return (FT_FALSE);
    if (filesystem_is_relative(path) == FT_FALSE)
        return (FT_FALSE);
    normalized_path = filesystem_normalize_path(path);
    if (normalized_path == ft_nullptr)
        return (FT_FALSE);
    if (normalized_path->get_error() != FT_ERR_SUCCESS)
        result = FT_FALSE;
    else if (filesystem_path_is_parent_reference(normalized_path->c_str())
        == FT_TRUE)
        result = FT_FALSE;
    else
        result = FT_TRUE;
    (void)normalized_path->destroy();
    delete normalized_path;
    return (result);
}
