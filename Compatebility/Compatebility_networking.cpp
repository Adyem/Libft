#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Networking/socket_class.hpp"

ssize_t cmp_socket_send_all(ft_socket *socket_object, const void *buffer,
                            size_t length, int flags)
{
    if (socket_object == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return (socket_object->send_all(buffer, length, flags));
}
