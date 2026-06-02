#include "cross_process.hpp"

#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_cross_process.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t cp_receive_descriptor(int32_t socket_file_descriptor,
    cross_process_message &message)
{
    int32_t descriptor_result;

    descriptor_result = cmp_cross_process_receive_descriptor(
            socket_file_descriptor, message);
    if (descriptor_result != FT_ERR_SUCCESS)
        return (descriptor_result);
    return (FT_ERR_SUCCESS);
}
