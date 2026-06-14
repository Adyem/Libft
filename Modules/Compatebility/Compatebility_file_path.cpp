#include "compatebility_internal.hpp"

#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstring>
#if defined(_WIN32) || defined(_WIN64)
char cmp_path_separator(void)
{
    return ('\\');
}

void cmp_normalize_slashes(char *data)
{
    if (!data)
        return ;
    ft_size_t index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '/')
            data[index] = '\\';
        index++;
    }
    return ;
}

static char *cmp_duplicate_path_string(const char *source_path)
{
    char *output_path;
    ft_size_t source_length;
    ft_size_t index;

    if (source_path == ft_nullptr)
        return (ft_nullptr);
    source_length = ft_strlen_size_t(source_path);
    output_path = static_cast<char *>(cma_malloc(source_length + 1));
    if (output_path == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < source_length)
    {
        output_path[index] = source_path[index];
        index++;
    }
    output_path[source_length] = '\0';
    return (output_path);
}

int32_t cmp_translate_path_to_native(const char *path, char **output_path)
{
    char *translated_path;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    translated_path = cmp_duplicate_path_string(path);
    if (translated_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
#if defined(_WIN32) || defined(_WIN64)
    if (translated_path[0] == '/' || translated_path[0] == '\\')
    {
        std::memmove(translated_path,
            translated_path + 1,
            std::strlen(translated_path));
    }
#endif
    cmp_normalize_slashes(translated_path);
    *output_path = translated_path;
    return (FT_ERR_SUCCESS);
}
#else
char cmp_path_separator(void)
{
    return ('/');
}

void cmp_normalize_slashes(char *data)
{
    if (!data)
        return ;
    ft_size_t index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '\\')
            data[index] = '/';
        index++;
    }
    return ;
}

static char *cmp_duplicate_path_string(const char *source_path)
{
    char *output_path;
    ft_size_t source_length;
    ft_size_t index;

    if (source_path == ft_nullptr)
        return (ft_nullptr);
    source_length = ft_strlen_size_t(source_path);
    output_path = static_cast<char *>(cma_malloc(source_length + 1));
    if (output_path == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < source_length)
    {
        output_path[index] = source_path[index];
        index++;
    }
    output_path[source_length] = '\0';
    return (output_path);
}

int32_t cmp_translate_path_to_native(const char *path, char **output_path)
{
    char *translated_path;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    translated_path = cmp_duplicate_path_string(path);
    if (translated_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    *output_path = translated_path;
    return (FT_ERR_SUCCESS);
}
#endif

#if defined(_WIN32) || defined(_WIN64)
static char cmp_path_lower_ascii(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character + ('a' - 'A')));
    return (character);
}
#endif

ft_bool cmp_path_equal(const char *path_left, const char *path_right) noexcept
{
    ft_size_t index;

    if (path_left == ft_nullptr || path_right == ft_nullptr)
        return (FT_FALSE);
    index = 0;
    while (path_left[index] != '\0' && path_right[index] != '\0')
    {
#if defined(_WIN32) || defined(_WIN64)
        if (cmp_path_lower_ascii(path_left[index])
            != cmp_path_lower_ascii(path_right[index]))
            return (FT_FALSE);
#else
        if (path_left[index] != path_right[index])
            return (FT_FALSE);
#endif
        ++index;
    }
    if (path_left[index] != path_right[index])
        return (FT_FALSE);
    return (FT_TRUE);
}

