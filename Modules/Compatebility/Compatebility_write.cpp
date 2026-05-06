#include "compatebility_internal.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>

int32_t cmp_su_write(int32_t file_descriptor, const char *buffer,
    ft_size_t length, int64_t *bytes_written_out)
{
    int64_t bytes_written_value;
    int32_t error_code;

    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = 0;
    errno = 0;
    bytes_written_value = su_write(file_descriptor, buffer, length);
    if (bytes_written_value < 0)
    {
        error_code = cmp_map_system_error_to_ft(errno);
        if (error_code == FT_ERR_SUCCESS)
            return (FT_ERR_IO);
        return (error_code);
    }
    if (bytes_written_out != ft_nullptr)
        *bytes_written_out = bytes_written_value;
    return (FT_ERR_SUCCESS);
}
