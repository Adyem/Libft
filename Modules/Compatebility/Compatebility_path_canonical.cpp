#include "compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
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
    DWORD required_size;
    DWORD written_size;
    char *buffer;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr || path[0] == '\0')
        return (FT_ERR_INVALID_ARGUMENT);
    required_size = GetFullPathNameA(path, 0, ft_nullptr, ft_nullptr);
    if (required_size == 0)
        return (FT_ERR_INVALID_PATH);
    buffer = static_cast<char *>(cma_malloc(
            static_cast<ft_size_t>(required_size) + 1));
    if (buffer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    written_size = GetFullPathNameA(path, required_size + 1, buffer,
            ft_nullptr);
    if (written_size == 0 || written_size > required_size)
    {
        cma_free(buffer);
        return (FT_ERR_INVALID_PATH);
    }
    cmp_normalize_slashes(buffer);
    *output_path = buffer;
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
