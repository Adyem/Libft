#include "cross_process.hpp"

#include <cstring>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Compatebility/compatebility_cross_process.hpp"

namespace
{
    static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
    {
        if (pointer_value < base_value)
            return (0);
        return (pointer_value - base_value);
    }
}

int32_t cp_receive_memory(int32_t socket_file_descriptor,
    cross_process_read_result &result)
{
    cross_process_message message;
    cmp_cross_process_mapping mapping;
    cmp_cross_process_mutex_state mutex_state;
    ft_size_t data_offset;
    ft_size_t payload_length;
    auto cleanup_and_fail = [&](int32_t error_code) -> int32_t {
        int32_t cleanup_error;

        cleanup_error = cmp_cross_process_unlock_mutex(message, &mapping,
                &mutex_state);
        if (cleanup_error != FT_ERR_SUCCESS)
            return (cleanup_error);
        cleanup_error = cmp_cross_process_close_mapping(&mapping);
        if (cleanup_error != FT_ERR_SUCCESS)
            return (cleanup_error);
        return (error_code);
    };
    auto reset_string = [](ft_string &value) -> int32_t {
        (void)value.destroy();
        return (value.initialize());
    };
    int32_t operation_error;
    int32_t assign_error;
    int32_t cleanup_error;

    mapping.mapping_address = ft_nullptr;
    mapping.mapping_length = 0;
    mapping.platform_handle = ft_nullptr;
    mapping.mutex_address = ft_nullptr;
    mutex_state.platform_mutex = ft_nullptr;
    operation_error = cmp_cross_process_receive_descriptor(
            socket_file_descriptor, message);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = cmp_cross_process_open_mapping(message, &mapping);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    operation_error = cmp_cross_process_lock_mutex(message, &mapping,
            &mutex_state);
    if (operation_error != FT_ERR_SUCCESS)
    {
        cleanup_error = cmp_cross_process_close_mapping(&mapping);
        if (cleanup_error != FT_ERR_SUCCESS)
            return (cleanup_error);
        return (operation_error);
    }
    if (reset_string(result.shared_memory_name) != FT_ERR_SUCCESS)
        return (cleanup_and_fail(FT_ERR_NO_MEMORY));
    result.shared_memory_name = message.shared_memory_name;
    data_offset = compute_offset(message.remote_memory_address, message.stack_base_address);
    if (data_offset >= message.remote_memory_size)
    {
        return (cleanup_and_fail(FT_ERR_INVALID_ARGUMENT));
    }
    payload_length = message.remote_memory_size - data_offset;
    if (reset_string(result.payload) != FT_ERR_SUCCESS)
        return (cleanup_and_fail(FT_ERR_NO_MEMORY));
    assign_error = result.payload.assign(reinterpret_cast<char *>(
                mapping.mapping_address + data_offset), payload_length);
    if (assign_error != FT_ERR_SUCCESS)
        return (cleanup_and_fail(assign_error));
    if (message.error_memory_address != 0)
    {
        ft_size_t error_offset;

        error_offset = compute_offset(message.error_memory_address, message.stack_base_address);
        if (error_offset + sizeof(int32_t) > message.remote_memory_size)
            return (cleanup_and_fail(FT_ERR_INVALID_ARGUMENT));
        std::memset(mapping.mapping_address + error_offset, 0, sizeof(int32_t));
    }
    std::memset(mapping.mapping_address + data_offset, 0, payload_length);
    operation_error = cmp_cross_process_unlock_mutex(message, &mapping,
            &mutex_state);
    if (operation_error != FT_ERR_SUCCESS)
    {
        cleanup_error = cmp_cross_process_close_mapping(&mapping);
        if (cleanup_error != FT_ERR_SUCCESS)
            return (cleanup_error);
        return (operation_error);
    }
    operation_error = cmp_cross_process_close_mapping(&mapping);
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    return (FT_ERR_SUCCESS);
}
