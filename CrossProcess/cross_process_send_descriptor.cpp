#include "cross_process.hpp"

#include <cerrno>

#include "../Compatebility/compatebility_cross_process.hpp"
#include "../Libft/libft.hpp"

int cp_send_descriptor(int socket_fd, const cross_process_message &message)
{
    return (cmp_cross_process_send_descriptor(socket_fd, message));
}
