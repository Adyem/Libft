#include "encryption.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"

int32_t encryption_secure_wipe(void *buffer, ft_size_t buffer_size)
{
    int32_t secure_error;

    if (buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (buffer_size == 0)
        return (FT_ERR_SUCCESS);
    secure_error = cmp_secure_memzero(buffer, buffer_size);
    if (secure_error != FT_ERR_SUCCESS)
        return (secure_error);
    return (FT_ERR_SUCCESS);
}

int32_t encryption_secure_wipe_string(char *string_buffer)
{
    ft_size_t string_length;
    ft_size_t wipe_length;
    int32_t wipe_error;

    if (string_buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    string_length = ft_strlen(string_buffer);
    wipe_length = string_length + 1;
    if (wipe_length <= string_length)
        return (FT_ERR_OUT_OF_RANGE);
    wipe_error = encryption_secure_wipe(string_buffer, wipe_length);
    if (wipe_error != FT_ERR_SUCCESS)
        return (wipe_error);
    return (FT_ERR_SUCCESS);
}
