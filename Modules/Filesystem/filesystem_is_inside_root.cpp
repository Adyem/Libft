#include "filesystem.hpp"
#include "../File/file_utils.hpp"

ft_bool filesystem_is_inside_root(const char *root_path,
    const char *candidate_path) noexcept
{
    return (file_path_is_inside_root(root_path, candidate_path));
}
