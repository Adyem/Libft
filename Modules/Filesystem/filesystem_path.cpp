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

ft_bool filesystem_is_rooted(const char *path) noexcept
{
    return (filesystem_is_absolute(path));
}

static ft_bool filesystem_ascii_case_equal_span(const char *left,
    ft_size_t length, const char *right) noexcept
{
    ft_size_t index;
    char left_character;
    char right_character;

    if (left == ft_nullptr || right == ft_nullptr)
        return (FT_FALSE);
    index = 0;
    while (index < length && right[index] != '\0')
    {
        left_character = left[index];
        right_character = right[index];
        if (left_character >= 'A' && left_character <= 'Z')
            left_character = static_cast<char>(left_character - 'A' + 'a');
        if (right_character >= 'A' && right_character <= 'Z')
            right_character = static_cast<char>(right_character - 'A' + 'a');
        if (left_character != right_character)
            return (FT_FALSE);
        index++;
    }
    if (index != length || right[index] != '\0')
        return (FT_FALSE);
    return (FT_TRUE);
}

int32_t filesystem_split_path(const char *path, ft_string *directory_out,
    ft_string *basename_out)
{
    ft_string *directory_value;
    ft_string *basename_value;
    int32_t error_code;

    if (directory_out == ft_nullptr || basename_out == ft_nullptr
        || directory_out == basename_out)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    directory_value = filesystem_dirname(path);
    if (directory_value == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    basename_value = filesystem_basename(path);
    if (basename_value == ft_nullptr)
    {
        (void)directory_value->destroy();
        delete directory_value;
        return (FT_ERR_NO_MEMORY);
    }
    (void)directory_out->destroy();
    (void)basename_out->destroy();
    error_code = directory_out->initialize(directory_value->c_str());
    if (error_code == FT_ERR_SUCCESS)
        error_code = basename_out->initialize(basename_value->c_str());
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)directory_out->destroy();
        (void)basename_out->destroy();
    }
    (void)directory_value->destroy();
    delete directory_value;
    (void)basename_value->destroy();
    delete basename_value;
    return (error_code);
}

ft_bool filesystem_is_hidden(const char *path) noexcept
{
    char *basename;
    ft_bool result;

    basename = file_path_basename(path);
    if (basename == ft_nullptr)
        return (FT_FALSE);
    result = FT_FALSE;
    if (basename[0] == '.'
        && basename[1] != '\0'
        && !(basename[1] == '.' && basename[2] == '\0'))
        result = FT_TRUE;
    cma_free(basename);
    return (result);
}

ft_bool filesystem_is_reserved_name(const char *path) noexcept
{
    char *basename;
    ft_size_t basename_length;
    ft_size_t trimmed_length;
    ft_size_t stem_length;
    ft_bool result;

    basename = file_path_basename(path);
    if (basename == ft_nullptr)
        return (FT_FALSE);
    basename_length = 0;
    while (basename[basename_length] != '\0')
        basename_length++;
    trimmed_length = basename_length;
    while (trimmed_length > 0
        && (basename[trimmed_length - 1] == '.'
            || basename[trimmed_length - 1] == ' '))
    {
        trimmed_length--;
    }
    stem_length = 0;
    while (stem_length < trimmed_length && basename[stem_length] != '.')
        stem_length++;
    result = FT_FALSE;
    if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "CON") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "PRN") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "AUX") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 3 && filesystem_ascii_case_equal_span(basename, 3,
            "NUL") == FT_TRUE)
        result = FT_TRUE;
    else if (stem_length == 4 && basename[0] == 'C'
        && (basename[1] == 'O' || basename[1] == 'o')
        && (basename[2] == 'M' || basename[2] == 'm')
        && basename[3] >= '1' && basename[3] <= '9')
    {
        result = FT_TRUE;
    }
    else if (stem_length == 4 && basename[0] == 'L'
        && (basename[1] == 'P' || basename[1] == 'p')
        && (basename[2] == 'T' || basename[2] == 't')
        && basename[3] >= '1' && basename[3] <= '9')
    {
        result = FT_TRUE;
    }
    cma_free(basename);
    return (result);
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
