#include "encryption_secure_wipe.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int encryption_secure_wipe(void *buffer, std::size_t buffer_size)
{
    int secure_error;

    if (buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (buffer_size == 0)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    secure_error = cmp_secure_memzero(buffer, buffer_size);
    if (secure_error != 0)
        return (-1);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int encryption_secure_wipe_string(char *string_buffer)
{
    std::size_t string_length;
    std::size_t wipe_length;

    if (string_buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    string_length = ft_strlen(string_buffer);
    wipe_length = string_length + 1;
    if (wipe_length <= string_length)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (encryption_secure_wipe(string_buffer, wipe_length) != 0)
        return (-1);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
