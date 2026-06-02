#include "filesystem.hpp"
#include "../File/file_utils.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t filesystem_validate_inside_root(const char *root_path,
    const char *candidate_path)
{
    return (file_validate_path_inside_root(root_path, candidate_path));
}
