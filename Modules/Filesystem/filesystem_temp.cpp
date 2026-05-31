#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../File/file_utils.hpp"
#include "../Time/time.hpp"
#include <cstdlib>

static thread_local uint64_t g_filesystem_temp_counter = 0U;

static const char *filesystem_temp_directory(void)
{
    const char *directory;

    directory = std::getenv("TMPDIR");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        return (directory);
    }
    directory = std::getenv("TEMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        return (directory);
    }
    directory = std::getenv("TMP");
    if (directory != ft_nullptr && directory[0] != '\0')
    {
        return (directory);
    }
    return ("/tmp");
}

static int32_t filesystem_append_uint64(ft_string *string_value, uint64_t value)
{
    char digits[21];
    ft_size_t digit_count;
    ft_size_t index;

    if (string_value == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    digit_count = 0;
    if (value == 0U)
    {
        digits[digit_count] = '0';
        digit_count++;
    }
    while (value > 0U)
    {
        digits[digit_count] = static_cast<char>('0' + (value % 10U));
        value = value / 10U;
        digit_count++;
    }
    index = digit_count;
    while (index > 0)
    {
        index--;
        if (string_value->append(digits[index]) != FT_ERR_SUCCESS)
        {
            return (string_value->get_error());
        }
    }
    return (FT_ERR_SUCCESS);
}

int32_t filesystem_temp_path(const char *prefix, const char *extension,
    ft_string *output)
{
    ft_string name;
    ft_string *joined_path;
    int32_t error_code;

    if (output == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    error_code = name.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    if (prefix != ft_nullptr && prefix[0] != '\0')
    {
        error_code = name.append(prefix);
    }
    else
    {
        error_code = name.append("libft");
    }
    if (error_code != FT_ERR_SUCCESS)
    {
        return (name.get_error());
    }
    if (name.append("_") != FT_ERR_SUCCESS)
    {
        return (name.get_error());
    }
    g_filesystem_temp_counter++;
    error_code = filesystem_append_uint64(&name,
            static_cast<uint64_t>(time_now_ms()) + g_filesystem_temp_counter);
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    if (extension != ft_nullptr && extension[0] != '\0')
    {
        if (extension[0] != '.')
        {
            error_code = name.append('.');
            if (error_code != FT_ERR_SUCCESS)
            {
                return (name.get_error());
            }
        }
        error_code = name.append(extension);
        if (error_code != FT_ERR_SUCCESS)
        {
            return (name.get_error());
        }
    }
    joined_path = file_path_join(filesystem_temp_directory(), name.c_str());
    if (joined_path == ft_nullptr)
    {
        return (FT_ERR_INVALID_STATE);
    }
    if (joined_path->get_error() != FT_ERR_SUCCESS)
    {
        error_code = joined_path->get_error();
        (void)joined_path->destroy();
        delete joined_path;
        return (error_code);
    }
    *output = *joined_path;
    (void)joined_path->destroy();
    delete joined_path;
    return (output->get_error());
}
