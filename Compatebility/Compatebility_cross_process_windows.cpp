#ifdef _WIN32

#include "compatebility_cross_process.hpp"

#include <cerrno>
#include <winsock2.h>
#include <windows.h>

static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
{
    if (pointer_value < base_value)
        return (0);
    return (static_cast<ft_size_t>(pointer_value - base_value));
}

int cmp_cross_process_send_descriptor(int socket_fd, const cross_process_message &message)
{
    WSABUF buffer;
    DWORD bytes_sent;
    int result;

    buffer.buf = reinterpret_cast<char *>(const_cast<cross_process_message *>(&message));
    buffer.len = static_cast<ULONG>(sizeof(cross_process_message));
    bytes_sent = 0;
    result = WSASend(reinterpret_cast<SOCKET>(socket_fd), &buffer, 1, &bytes_sent, 0, 0, 0);
    if (result != 0 || bytes_sent != buffer.len)
    {
        int windows_error;

        windows_error = WSAGetLastError();
        errno = windows_error;
        return (-1);
    }
    return (0);
}

int cmp_cross_process_receive_descriptor(int socket_fd, cross_process_message &message)
{
    char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<char *>(&message);
    total_size = static_cast<ft_size_t>(sizeof(cross_process_message));
    offset = 0;
    while (offset < total_size)
    {
        int chunk_size;

        chunk_size = recv(reinterpret_cast<SOCKET>(socket_fd), raw_message + offset, static_cast<int>(total_size - offset), 0);
        if (chunk_size == SOCKET_ERROR)
        {
            int windows_error;

            windows_error = WSAGetLastError();
            errno = static_cast<int>(windows_error);
            return (-1);
        }
        if (chunk_size == 0)
        {
            errno = ECONNRESET;
            return (-1);
        }
        offset += static_cast<ft_size_t>(chunk_size);
    }
    return (0);
}

int cmp_cross_process_open_mapping(const cross_process_message &message, cmp_cross_process_mapping *mapping)
{
    HANDLE mapping_handle;
    void *mapping_pointer;
    DWORD windows_error;

    mapping_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, message.shared_memory_name);
    if (mapping_handle == NULL)
    {
        windows_error = GetLastError();
        errno = static_cast<int>(windows_error);
        return (-1);
    }
    mapping_pointer = MapViewOfFile(mapping_handle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<SIZE_T>(message.remote_memory_size));
    CloseHandle(mapping_handle);
    if (mapping_pointer == ft_nullptr)
    {
        windows_error = GetLastError();
        errno = static_cast<int>(windows_error);
        return (-1);
    }
    mapping->mapping_address = reinterpret_cast<unsigned char *>(mapping_pointer);
    mapping->mapping_length = static_cast<ft_size_t>(message.remote_memory_size);
    mapping->platform_handle = ft_nullptr;
    return (0);
}

int cmp_cross_process_close_mapping(cmp_cross_process_mapping *mapping)
{
    if (mapping->mapping_address == ft_nullptr)
        return (0);
    if (UnmapViewOfFile(mapping->mapping_address) == 0)
    {
        DWORD windows_error;

        windows_error = GetLastError();
        errno = static_cast<int>(windows_error);
        return (-1);
    }
    mapping->mapping_address = ft_nullptr;
    mapping->mapping_length = 0;
    mapping->platform_handle = ft_nullptr;
    return (0);
}

int cmp_cross_process_lock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    ft_size_t mutex_offset;
    HANDLE shared_mutex_handle;
    DWORD wait_result;
    DWORD windows_error;
    int attempt_count;
    bool mutex_locked;

    mutex_offset = compute_offset(message.shared_mutex_address, message.stack_base_address);
    if (mutex_offset + static_cast<ft_size_t>(sizeof(HANDLE)) > mapping->mapping_length)
    {
        errno = EINVAL;
        return (-1);
    }
    shared_mutex_handle = *reinterpret_cast<HANDLE *>(mapping->mapping_address + mutex_offset);
    if (shared_mutex_handle == NULL)
    {
        errno = EINVAL;
        return (-1);
    }
    attempt_count = 0;
    mutex_locked = false;
    while (attempt_count < 5)
    {
        wait_result = WaitForSingleObject(shared_mutex_handle, 0);
        if (wait_result == WAIT_OBJECT_0 || wait_result == WAIT_ABANDONED)
        {
            mutex_locked = true;
            break;
        }
        if (wait_result == WAIT_TIMEOUT)
        {
            attempt_count += 1;
            if (attempt_count >= 5)
                break;
            Sleep(50);
            continue;
        }
        windows_error = GetLastError();
        errno = static_cast<int>(windows_error);
        return (-1);
    }
    if (mutex_locked == false)
    {
        errno = ETIMEDOUT;
        return (-1);
    }
    mutex_state->platform_mutex = shared_mutex_handle;
    return (0);
}

int cmp_cross_process_unlock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    HANDLE shared_mutex_handle;
    DWORD windows_error;

    (void)message;
    (void)mapping;
    shared_mutex_handle = reinterpret_cast<HANDLE>(mutex_state->platform_mutex);
    if (shared_mutex_handle == NULL)
        return (0);
    if (ReleaseMutex(shared_mutex_handle) == 0)
    {
        windows_error = GetLastError();
        errno = static_cast<int>(windows_error);
        return (-1);
    }
    mutex_state->platform_mutex = ft_nullptr;
    return (0);
}

#endif
