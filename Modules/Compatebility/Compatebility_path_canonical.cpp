#include "compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if !defined(_WIN32) && !defined(_WIN64)
# include <cstdlib>
#endif

static char *cmp_path_duplicate_c_string(const char *source)
{
    char *output;
    ft_size_t size;
    ft_size_t index;

    if (source == ft_nullptr)
        return (ft_nullptr);
    size = ft_strlen_size_t(source);
    output = static_cast<char *>(cma_malloc(size + 1));
    if (output == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < size)
    {
        output[index] = source[index];
        index++;
    }
    output[size] = '\0';
    return (output);
}

#if defined(_WIN32) || defined(_WIN64)
int32_t cmp_path_canonical(const char *path, char **output_path)
{
    DWORD written_size;
    char *current_directory;
    ft_string *joined_path;
    ft_string *normalized_path;
    char *canonical_path;
    ft_bool path_is_absolute;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr || path[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_get_type(path) == FILE_TYPE_MISSING)
        return (FT_ERR_NOT_FOUND);
    path_is_absolute = file_path_is_absolute(path);
    if (path_is_absolute == FT_TRUE)
        normalized_path = file_path_normalize(path);
    else
    {
        current_directory = static_cast<char *>(cma_malloc(32768U));
        if (current_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        written_size = GetCurrentDirectoryA(32768U, current_directory);
        if (written_size == 0)
        {
            cma_free(current_directory);
            return (FT_ERR_INVALID_PATH);
        }
        if (written_size >= 32768U)
        {
            cma_free(current_directory);
            return (FT_ERR_PATH_TOO_LONG);
        }
        joined_path = file_path_join(current_directory, path);
        cma_free(current_directory);
        if (joined_path == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        if (joined_path->get_error() != FT_ERR_SUCCESS)
        {
            int32_t joined_error;

            joined_error = joined_path->get_error();
            (void)joined_path->destroy();
            delete joined_path;
            return (joined_error);
        }
        normalized_path = joined_path;
    }
    if (normalized_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (normalized_path->get_error() != FT_ERR_SUCCESS)
    {
        int32_t normalization_error;

        normalization_error = normalized_path->get_error();
        (void)normalized_path->destroy();
        delete normalized_path;
        return (normalization_error);
    }
    canonical_path = cmp_path_duplicate_c_string(normalized_path->c_str());
    (void)normalized_path->destroy();
    delete normalized_path;
    if (canonical_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    *output_path = canonical_path;
    return (FT_ERR_SUCCESS);
}
#else
int32_t cmp_path_canonical(const char *path, char **output_path)
{
    char *system_path;
    char *canonical_path;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr || path[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    system_path = realpath(path, ft_nullptr);
    if (system_path == ft_nullptr)
        return (FT_ERR_NOT_FOUND);
    canonical_path = cmp_path_duplicate_c_string(system_path);
    free(system_path);
    if (canonical_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    cmp_normalize_slashes(canonical_path);
    *output_path = canonical_path;
    return (FT_ERR_SUCCESS);
}
#endif
