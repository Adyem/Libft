#include "libft_config.hpp"

#if LIBFT_ENABLE_FILE_IO_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>
#include <cerrno>

char *ft_fgets(char *string, int size, FILE *stream)
{
    char *result_string;
    int error_code;

    if (string == ft_nullptr || stream == ft_nullptr || size <= 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    result_string = std::fgets(string, size, stream);
    if (result_string == ft_nullptr)
    {
        if (std::ferror(stream) != 0)
        {
            int saved_errno;

            saved_errno = errno;
            if (saved_errno != 0)
                error_code = cmp_map_system_error_to_ft(saved_errno);
            else
                error_code = FT_ERR_INVALID_HANDLE;
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        if (std::feof(stream) != 0)
        {
            ft_global_error_stack_push(FT_ERR_END_OF_FILE);
            return (ft_nullptr);
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result_string);
}
#endif
