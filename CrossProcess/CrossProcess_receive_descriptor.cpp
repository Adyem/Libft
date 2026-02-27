#include "cross_process.hpp"

#include "../Compatebility/compatebility_cross_process.hpp"

int cp_receive_descriptor(int socket_file_descriptor, cross_process_message &message)
{
    int descriptor_result = cmp_cross_process_receive_descriptor(socket_file_descriptor, message);
    if (descriptor_result != 0)
        return (-1);
    return (0);
}
