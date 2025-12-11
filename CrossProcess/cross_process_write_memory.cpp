#include "cross_process.hpp"

#include <cerrno>
#include <cstring>
#include "../CPP_class/class_nullptr.hpp"
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

int cp_write_memory(const cross_process_message &message, const unsigned char *payload, ft_size_t payload_length, int error_code)
{
    ft_size_t data_offset;
    ft_size_t payload_capacity;
    ft_size_t error_offset;
    ft_size_t zero_length;
    ft_size_t full_capacity;
    bool has_error_slot;
    bool has_failure;
    bool zero_on_failure;
    int failure_errno;

    if (payload == ft_nullptr && payload_length != 0)
    {
        errno = EINVAL;
        return (-1);
    }
    data_offset = compute_offset(message.remote_memory_address, message.stack_base_address);
    if (data_offset >= message.remote_memory_size)
    {
        errno = EINVAL;
        return (-1);
    }
    full_capacity = message.remote_memory_size - data_offset;
    payload_capacity = full_capacity;
    has_error_slot = message.error_memory_address != 0;
    has_failure = false;
    zero_on_failure = true;
    failure_errno = 0;
    error_offset = 0;
    if (has_error_slot)
    {
        error_offset = compute_offset(message.error_memory_address, message.stack_base_address);
        if (error_offset + static_cast<ft_size_t>(sizeof(int)) > message.remote_memory_size || error_offset < data_offset)
        {
            has_failure = true;
            failure_errno = EINVAL;
        }
        else
        {
            payload_capacity = error_offset - data_offset;
        }
    }
    if (!has_failure && payload_length > payload_capacity)
    {
        has_failure = true;
        failure_errno = ERANGE;
        zero_on_failure = false;
    }
    cmp_cross_process_mapping mapping;
    cmp_cross_process_mutex_state mutex_state;

    mapping.mapping_address = ft_nullptr;
    mapping.mapping_length = 0;
    mapping.platform_handle = ft_nullptr;
    mutex_state.platform_mutex = ft_nullptr;
    if (cmp_cross_process_open_mapping(message, &mapping) != 0)
        return (-1);
    if (cmp_cross_process_lock_mutex(message, &mapping, &mutex_state) != 0)
    {
        cmp_cross_process_close_mapping(&mapping);
        return (-1);
    }
    zero_length = payload_capacity;
    if (zero_length > 0 && (!has_failure || zero_on_failure))
        std::memset(mapping.mapping_address + data_offset, 0, zero_length);
    if (!has_failure && payload_length > 0)
        std::memcpy(mapping.mapping_address + data_offset, payload, payload_length);
    if (!has_failure && has_error_slot)
    {
        std::memcpy(mapping.mapping_address + error_offset, &error_code, sizeof(int));
    }
    if (cmp_cross_process_unlock_mutex(message, &mapping, &mutex_state) != 0)
    {
        cmp_cross_process_close_mapping(&mapping);
        return (-1);
    }
    if (cmp_cross_process_close_mapping(&mapping) != 0)
        return (-1);
    if (has_failure)
    {
        errno = failure_errno;
        return (-1);
    }
    return (0);
}
