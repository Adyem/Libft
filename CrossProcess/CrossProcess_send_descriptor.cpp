#include "cross_process.hpp"

#include "../Compatebility/compatebility_cross_process.hpp"

int cp_send_descriptor(int socket_fd, const cross_process_message &message)
{
    int descriptor_result = cmp_cross_process_send_descriptor(socket_fd, message);
    if (descriptor_result != 0)
        return (-1);
    return (0);
}
