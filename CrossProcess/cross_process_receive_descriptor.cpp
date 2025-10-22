#include "cross_process.hpp"

#include "../Compatebility/compatebility_cross_process.hpp"
#include "../Libft/libft.hpp"

int cp_receive_descriptor(int socket_fd, cross_process_message &message)
{
    return (cmp_cross_process_receive_descriptor(socket_fd, message));
}
