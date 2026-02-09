#include "cross_process.hpp"

#include <cerrno>
#include "../Compatebility/compatebility_cross_process.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"

int cp_receive_descriptor(int socket_fd, cross_process_message &message)
{
    int descriptor_result = cmp_cross_process_receive_descriptor(socket_fd, message);
    if (descriptor_result != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
