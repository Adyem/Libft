#include "filesystem.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../File/file_utils.hpp"
#include "../Time/time.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static thread_local uint64_t g_filesystem_temp_counter = 0U;

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
    char *native_directory_path;
    char *portable_path;
    int32_t error_code;

    if (output == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (output->is_initialised() == FT_TRUE)
    {
        error_code = output->destroy();
        if (error_code != FT_ERR_SUCCESS)
        {
            return (error_code);
        }
    }
    error_code = output->initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        return (error_code);
    }
    error_code = name.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)output->destroy();
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
        (void)output->destroy();
        return (name.get_error());
    }
    if (name.append("_") != FT_ERR_SUCCESS)
    {
        (void)output->destroy();
        return (name.get_error());
    }
    g_filesystem_temp_counter++;
    error_code = filesystem_append_uint64(&name,
            static_cast<uint64_t>(time_now_ms()) + g_filesystem_temp_counter);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)output->destroy();
        return (error_code);
    }
    if (extension != ft_nullptr && extension[0] != '\0')
    {
        if (extension[0] != '.')
        {
            error_code = name.append('.');
            if (error_code != FT_ERR_SUCCESS)
            {
                (void)output->destroy();
                return (name.get_error());
            }
        }
        error_code = name.append(extension);
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)output->destroy();
            return (name.get_error());
        }
    }
    native_directory_path = ft_nullptr;
    error_code = cmp_get_temp_directory(&native_directory_path);
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)output->destroy();
        return (error_code);
    }
    joined_path = file_path_join(native_directory_path, name.c_str());
    cma_free(native_directory_path);
    if (joined_path == ft_nullptr)
    {
        (void)output->destroy();
        return (FT_ERR_INVALID_STATE);
    }
    if (joined_path->get_error() != FT_ERR_SUCCESS)
    {
        error_code = joined_path->get_error();
        (void)joined_path->destroy();
        delete joined_path;
        (void)output->destroy();
        return (error_code);
    }
    portable_path = ft_nullptr;
    error_code = cmp_translate_path_to_portable(joined_path->c_str(), &portable_path);
    if (error_code == FT_ERR_SUCCESS)
    {
        ft_size_t index;

        index = 0;
        while (portable_path[index] != '\0')
        {
            if (portable_path[index] == '\\')
                portable_path[index] = '/';
            index++;
        }
        error_code = output->assign(portable_path, ft_strlen_size_t(portable_path));
    }
    cma_free(portable_path);
    (void)joined_path->destroy();
    delete joined_path;
    if (error_code != FT_ERR_SUCCESS)
        (void)output->destroy();
    return (error_code);
}
