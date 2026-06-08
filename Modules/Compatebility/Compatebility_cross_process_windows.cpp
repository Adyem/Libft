#ifdef _WIN32

#include "compatebility_cross_process.hpp"
#include "compatebility_internal.hpp"
#include <cerrno>
#include <winsock2.h>
#include <windows.h>

static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
{
    if (pointer_value < base_value)
        return (0);
    return (pointer_value - base_value);
}

int32_t cmp_cross_process_send_descriptor(int32_t socket_file_descriptor, const cross_process_message &message)
{
    WSABUF buffer;
    DWORD bytes_sent;
    int32_t result;

    buffer.buf = reinterpret_cast<char *>(const_cast<cross_process_message *>(&message));
    buffer.len = static_cast<ULONG>(sizeof(cross_process_message));
    bytes_sent = 0;
    result = WSASend(static_cast<SOCKET>(socket_file_descriptor), &buffer, 1, &bytes_sent, 0, ft_nullptr, ft_nullptr);
    if (result != 0 || bytes_sent != buffer.len)
    {
        int32_t windows_error;

        windows_error = WSAGetLastError();
        errno = windows_error;
        return (cmp_map_system_error_to_ft(errno));
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cross_process_receive_descriptor(int32_t socket_file_descriptor, cross_process_message &message)
{
    char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<char *>(&message);
    total_size = sizeof(cross_process_message);
    offset = 0;
    while (offset < total_size)
    {
        int32_t chunk_size;

        chunk_size = recv(static_cast<SOCKET>(socket_file_descriptor), raw_message + offset, static_cast<int32_t>(total_size - offset), 0);
        if (chunk_size == SOCKET_ERROR)
        {
            int32_t windows_error;

            windows_error = WSAGetLastError();
            errno = static_cast<int32_t>(windows_error);
                return (cmp_map_system_error_to_ft(errno));
        }
        if (chunk_size == 0)
        {
            errno = ECONNRESET;
                return (cmp_map_system_error_to_ft(errno));
        }
        offset += static_cast<ft_size_t>(chunk_size);
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cross_process_open_mapping(const cross_process_message &message, cmp_cross_process_mapping *mapping)
{
    HANDLE mapping_handle;
    void *mapping_pointer;
    DWORD windows_error;

    mapping_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, message.shared_memory_name);
    if (mapping_handle == NULL)
    {
        windows_error = GetLastError();
        errno = static_cast<int32_t>(windows_error);
        return (cmp_map_system_error_to_ft(errno));
    }
    mapping_pointer = MapViewOfFile(mapping_handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<SIZE_T>(message.remote_memory_size));
    CloseHandle(mapping_handle);
    if (mapping_pointer == ft_nullptr)
    {
        windows_error = GetLastError();
        errno = static_cast<int32_t>(windows_error);
        return (cmp_map_system_error_to_ft(errno));
    }
    mapping->mapping_address = reinterpret_cast<unsigned char *>(mapping_pointer);
    mapping->mapping_length = static_cast<ft_size_t>(message.remote_memory_size);
    mapping->platform_handle = ft_nullptr;
    mapping->mutex_address = ft_nullptr;
    if (message.shared_mutex_address == 0)
    {
        cmp_cross_process_close_mapping(mapping);
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    ft_size_t mutex_offset = compute_offset(message.shared_mutex_address, message.stack_base_address);
    if (mutex_offset + sizeof(HANDLE) > mapping->mapping_length)
    {
        cmp_cross_process_close_mapping(mapping);
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    mapping->mutex_address = mapping->mapping_address + mutex_offset;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cross_process_close_mapping(cmp_cross_process_mapping *mapping)
{
    if (mapping->mapping_address == ft_nullptr)
        return (FT_ERR_SUCCESS);
    if (UnmapViewOfFile(mapping->mapping_address) == 0)
    {
        DWORD windows_error;

        windows_error = GetLastError();
        errno = static_cast<int32_t>(windows_error);
        return (cmp_map_system_error_to_ft(errno));
    }
    mapping->mapping_address = ft_nullptr;
    mapping->mapping_length = 0;
    mapping->platform_handle = ft_nullptr;
    mapping->mutex_address = ft_nullptr;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cross_process_lock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    (void)message;
    if (!mapping || mapping->mutex_address == ft_nullptr)
    {
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    HANDLE shared_mutex_handle = *reinterpret_cast<HANDLE *>(mapping->mutex_address);
    if (shared_mutex_handle == NULL)
    {
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    int32_t attempt_count = 0;
    ft_bool mutex_locked = FT_FALSE;
    while (attempt_count < 5)
    {
        DWORD wait_result = WaitForSingleObject(shared_mutex_handle, 0);
        if (wait_result == WAIT_OBJECT_0 || wait_result == WAIT_ABANDONED)
        {
            mutex_locked = FT_TRUE;
            break ;
        }
        if (wait_result == WAIT_TIMEOUT)
        {
            attempt_count += 1;
            if (attempt_count >= 5)
                break ;
            Sleep(50);
            continue;
        }
        DWORD windows_error = GetLastError();
        errno = static_cast<int32_t>(windows_error);
        return (cmp_map_system_error_to_ft(errno));
    }
    if (mutex_locked == FT_FALSE)
    {
        errno = ETIMEDOUT;
        return (cmp_map_system_error_to_ft(errno));
    }
    mutex_state->platform_mutex = shared_mutex_handle;
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cross_process_unlock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    HANDLE shared_mutex_handle;
    DWORD windows_error;

    (void)message;
    (void)mapping;
    shared_mutex_handle = reinterpret_cast<HANDLE>(mutex_state->platform_mutex);
    if (shared_mutex_handle == NULL)
        return (FT_ERR_SUCCESS);
    if (ReleaseMutex(shared_mutex_handle) == 0)
    {
        windows_error = GetLastError();
        errno = static_cast<int32_t>(windows_error);
        return (cmp_map_system_error_to_ft(errno));
    }
    mutex_state->platform_mutex = ft_nullptr;
    return (FT_ERR_SUCCESS);
}

#endif
