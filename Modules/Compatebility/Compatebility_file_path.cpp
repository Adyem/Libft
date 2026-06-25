#include "compatebility_internal.hpp"

#include "../System_utils/system_utils.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

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

#if defined(_WIN32) || defined(_WIN64)
char cmp_path_separator(void)
{
    return ('\\');
}

void cmp_normalize_slashes(char *data)
{
    ft_size_t index;

    if (data == ft_nullptr)
        return ;
    index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '/')
            data[index] = '\\';
        index++;
    }
    return ;
}

int32_t cmp_translate_path_to_native(const char *path, char **output_path)
{
    char *translated_path;
    ft_size_t source_length;
    ft_size_t index;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    translated_path = cmp_duplicate_path_string(path);
    if (translated_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (translated_path[0] == '/' || translated_path[0] == '\\')
    {
        source_length = ft_strlen_size_t(translated_path);
        index = 0;
        while (index < source_length)
        {
            translated_path[index] = translated_path[index + 1];
            index++;
        }
        translated_path[source_length] = '\0';
    }
    cmp_normalize_slashes(translated_path);
    *output_path = translated_path;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_translate_path_to_portable(const char *path, char **output_path)
{
    char *translated_path;
    ft_size_t index;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    translated_path = cmp_duplicate_path_string(path);
    if (translated_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    index = 0;
    while (translated_path[index] != '\0')
    {
        if (translated_path[index] == '\\')
            translated_path[index] = '/';
        index++;
    }
    *output_path = translated_path;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_get_temp_directory(char **output_path)
{
    char *native_directory;
    const char *directory;
    char windows_temp_directory_path[MAX_PATH + 1];
    DWORD path_length;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    path_length = GetTempPathA(MAX_PATH + 1, windows_temp_directory_path);
    if (path_length > 0 && path_length <= MAX_PATH)
    {
        windows_temp_directory_path[path_length] = '\0';
        native_directory = cmp_duplicate_path_string(windows_temp_directory_path);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    directory = su_getenv("TMPDIR");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    directory = su_getenv("TEMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    directory = su_getenv("TMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    native_directory = cmp_duplicate_path_string(".");
    if (native_directory == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    *output_path = native_directory;
    return (FT_ERR_SUCCESS);
}
#else
char cmp_path_separator(void)
{
    return ('/');
}

void cmp_normalize_slashes(char *data)
{
    ft_size_t index;

    if (data == ft_nullptr)
        return ;
    index = 0;
    while (data[index] != '\0')
    {
        if (data[index] == '\\')
            data[index] = '/';
        index++;
    }
    return ;
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

int32_t cmp_translate_path_to_portable(const char *path, char **output_path)
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

int32_t cmp_get_temp_directory(char **output_path)
{
    char *native_directory;
    const char *directory;

    if (output_path == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    *output_path = ft_nullptr;
    directory = su_getenv("TMPDIR");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    directory = su_getenv("TEMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    directory = su_getenv("TMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        native_directory = cmp_duplicate_path_string(directory);
        if (native_directory == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        *output_path = native_directory;
        return (FT_ERR_SUCCESS);
    }
    native_directory = cmp_duplicate_path_string(".");
    if (native_directory == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    *output_path = native_directory;
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
        index++;
    }
    if (path_left[index] != path_right[index])
        return (FT_FALSE);
    return (FT_TRUE);
}
