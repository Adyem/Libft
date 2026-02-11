#ifndef _WIN32

#include "compatebility_cross_process.hpp"
#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
{
    if (pointer_value < base_value)
        return (0);
    return (pointer_value - base_value);
}

int32_t cmp_cross_process_send_descriptor(int32_t socket_fd, const cross_process_message &message)
{
    const unsigned char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<const unsigned char *>(&message);
    total_size = sizeof(cross_process_message);
    offset = 0;
    while (offset < total_size)
    {
        int64_t written;

        written = ::send(socket_fd, raw_message + offset,
                total_size - offset, 0);
        if (written < 0)
        {
            if (errno == EINTR)
                continue;
            return (cmp_map_system_error_to_ft(errno));
        }
        offset += static_cast<ft_size_t>(written);
    }
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_cross_process_receive_descriptor(int32_t socket_fd, cross_process_message &message)
{
    unsigned char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<unsigned char *>(&message);
    total_size = sizeof(cross_process_message);
    offset = 0;
    while (offset < total_size)
    {
        int64_t received;

        received = ::recv(socket_fd, raw_message + offset,
                total_size - offset, 0);
        if (received < 0)
        {
            if (errno == EINTR)
                continue;
            return (cmp_map_system_error_to_ft(errno));
        }
        if (received == 0)
        {
            errno = ECONNRESET;
            return (cmp_map_system_error_to_ft(errno));
        }
        offset += static_cast<ft_size_t>(received);
    }
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_cross_process_open_mapping(const cross_process_message &message, cmp_cross_process_mapping *mapping)
{
    int32_t shared_memory_fd;
    void *mapping_pointer;

    shared_memory_fd = shm_open(message.shared_memory_name, O_RDWR, 0600);
    if (shared_memory_fd < 0)
        return (cmp_map_system_error_to_ft(errno));
    mapping_pointer = mmap(ft_nullptr, message.remote_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    ::close(shared_memory_fd);
    if (mapping_pointer == MAP_FAILED)
        return (cmp_map_system_error_to_ft(errno));
    mapping->mapping_address = reinterpret_cast<unsigned char *>(mapping_pointer);
    mapping->mapping_length = message.remote_memory_size;
    mapping->platform_handle = ft_nullptr;
    mapping->mutex_address = ft_nullptr;
    if (message.shared_mutex_address == 0)
    {
        cmp_cross_process_close_mapping(mapping);
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    ft_size_t mutex_offset = compute_offset(message.shared_mutex_address, message.stack_base_address);
    if (mutex_offset + sizeof(pthread_mutex_t) > mapping->mapping_length)
    {
        cmp_cross_process_close_mapping(mapping);
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    mapping->mutex_address = mapping->mapping_address + mutex_offset;
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_cross_process_close_mapping(cmp_cross_process_mapping *mapping)
{
    if (mapping->mapping_address == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    if (munmap(mapping->mapping_address, mapping->mapping_length) != 0)
        return (cmp_map_system_error_to_ft(errno));
    mapping->mapping_address = ft_nullptr;
    mapping->mapping_length = 0;
    mapping->platform_handle = ft_nullptr;
    mapping->mutex_address = ft_nullptr;
    return (FT_ERR_SUCCESSS);
}

int32_t cmp_cross_process_lock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    (void)message;
    if (!mapping || mapping->mutex_address == ft_nullptr)
    {
        errno = EINVAL;
        return (cmp_map_system_error_to_ft(errno));
    }
    pthread_mutex_t *shared_mutex = reinterpret_cast<pthread_mutex_t *>(mapping->mutex_address);
    for (int32_t attempt_count = 0; attempt_count < 5; ++attempt_count)
    {
        int32_t lock_error = pthread_mutex_trylock(shared_mutex);
        if (lock_error == 0)
        {
            mutex_state->platform_mutex = shared_mutex;
            return (FT_ERR_SUCCESSS);
        }
        if (lock_error != EBUSY)
        {
            errno = lock_error;
            return (cmp_map_system_error_to_ft(errno));
        }
        if (attempt_count >= 4)
            break;
        usleep(50000);
    }
    errno = ETIMEDOUT;
    return (cmp_map_system_error_to_ft(errno));
}

int32_t cmp_cross_process_unlock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    pthread_mutex_t *shared_mutex;

    (void)message;
    (void)mapping;
    shared_mutex = reinterpret_cast<pthread_mutex_t *>(mutex_state->platform_mutex);
    if (shared_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int32_t unlock_error = pthread_mutex_unlock(shared_mutex);
    if (unlock_error != 0)
    {
        errno = unlock_error;
        return (cmp_map_system_error_to_ft(errno));
    }
    mutex_state->platform_mutex = ft_nullptr;
    return (FT_ERR_SUCCESSS);
}

#endif
