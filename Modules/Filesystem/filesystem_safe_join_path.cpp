#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

ft_string *filesystem_safe_join_path(const char *root_path,
    const char *relative_path)
{
    ft_string *joined_path;

    if (root_path == ft_nullptr || relative_path == ft_nullptr)
        return (ft_nullptr);
    if (filesystem_is_safe_relative_path(relative_path) == FT_FALSE)
        return (ft_nullptr);
    joined_path = filesystem_join_path(root_path, relative_path);
    if (joined_path == ft_nullptr)
        return (ft_nullptr);
    if (joined_path->get_error() != FT_ERR_SUCCESS)
    {
        (void)joined_path->destroy();
        delete joined_path;
        return (ft_nullptr);
    }
    if (filesystem_is_inside_root(root_path, joined_path->c_str()) == FT_FALSE)
    {
        (void)joined_path->destroy();
        delete joined_path;
        return (ft_nullptr);
    }
    return (joined_path);
}
