#include "cross_process.hpp"

#include <cerrno>
#include <cstring>

#include "../Compatebility/compatebility_cross_process.hpp"
#include "../Libft/libft.hpp"

namespace
{
    static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
    {
        if (pointer_value < base_value)
            return (0);
        return (static_cast<ft_size_t>(pointer_value - base_value));
    }
}

int cp_receive_memory(int socket_fd, cross_process_read_result &result)
{
    cross_process_message message;
    cmp_cross_process_mapping mapping;
    cmp_cross_process_mutex_state mutex_state;
    ft_size_t data_offset;
    ft_size_t payload_length;

    mapping.mapping_address = ft_nullptr;
    mapping.mapping_length = 0;
    mapping.platform_handle = ft_nullptr;
    mutex_state.platform_mutex = ft_nullptr;
    if (cmp_cross_process_receive_descriptor(socket_fd, message) != 0)
        return (-1);
    result.shared_memory_name = message.shared_memory_name;
    if (cmp_cross_process_open_mapping(message, &mapping) != 0)
        return (-1);
    if (cmp_cross_process_lock_mutex(message, &mapping, &mutex_state) != 0)
    {
        cmp_cross_process_close_mapping(&mapping);
        return (-1);
    }
    data_offset = compute_offset(message.remote_memory_address, message.stack_base_address);
    if (data_offset >= message.remote_memory_size)
    {
        if (cmp_cross_process_unlock_mutex(message, &mapping, &mutex_state) != 0)
        {
            cmp_cross_process_close_mapping(&mapping);
            return (-1);
        }
        if (cmp_cross_process_close_mapping(&mapping) != 0)
            return (-1);
        errno = EINVAL;
        return (-1);
    }
    payload_length = message.remote_memory_size - data_offset;
    result.payload.assign(reinterpret_cast<char *>(mapping.mapping_address + data_offset), payload_length);
    if (message.error_memory_address != 0)
    {
        ft_size_t error_offset;

        error_offset = compute_offset(message.error_memory_address, message.stack_base_address);
        if (error_offset + static_cast<ft_size_t>(sizeof(int)) > message.remote_memory_size)
        {
            if (cmp_cross_process_unlock_mutex(message, &mapping, &mutex_state) != 0)
            {
                cmp_cross_process_close_mapping(&mapping);
                return (-1);
            }
            if (cmp_cross_process_close_mapping(&mapping) != 0)
                return (-1);
            errno = EINVAL;
            return (-1);
        }
        std::memset(mapping.mapping_address + error_offset, 0, sizeof(int));
    }
    std::memset(mapping.mapping_address + data_offset, 0, payload_length);
    if (cmp_cross_process_unlock_mutex(message, &mapping, &mutex_state) != 0)
    {
        cmp_cross_process_close_mapping(&mapping);
        return (-1);
    }
    if (cmp_cross_process_close_mapping(&mapping) != 0)
        return (-1);
    return (0);
}
