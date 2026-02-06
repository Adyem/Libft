#ifndef _WIN32

#include "compatebility_cross_process.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread_internal.hpp"

#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include <sys/socket.h>
#include <unistd.h>

static ft_size_t compute_offset(uint64_t pointer_value, uint64_t base_value)
{
    if (pointer_value < base_value)
        return (0);
    return (static_cast<ft_size_t>(pointer_value - base_value));
}

int cmp_cross_process_send_descriptor(int socket_fd, const cross_process_message &message)
{
    const unsigned char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<const unsigned char *>(&message);
    total_size = static_cast<ft_size_t>(sizeof(cross_process_message));
    offset = 0;
    while (offset < total_size)
    {
        ssize_t written;

        written = ::send(socket_fd, raw_message + offset, total_size - offset, 0);
        if (written < 0)
        {
            if (errno == EINTR)
                continue;
            return (-1);
        }
        offset += static_cast<ft_size_t>(written);
    }
    return (0);
}

int cmp_cross_process_receive_descriptor(int socket_fd, cross_process_message &message)
{
    unsigned char *raw_message;
    ft_size_t total_size;
    ft_size_t offset;

    raw_message = reinterpret_cast<unsigned char *>(&message);
    total_size = static_cast<ft_size_t>(sizeof(cross_process_message));
    offset = 0;
    while (offset < total_size)
    {
        ssize_t received;

        received = ::recv(socket_fd, raw_message + offset, total_size - offset, 0);
        if (received < 0)
        {
            if (errno == EINTR)
                continue;
            return (-1);
        }
        if (received == 0)
        {
            errno = ECONNRESET;
            return (-1);
        }
        offset += static_cast<ft_size_t>(received);
    }
    return (0);
}

int cmp_cross_process_open_mapping(const cross_process_message &message, cmp_cross_process_mapping *mapping)
{
    int shared_memory_fd;
    void *mapping_pointer;

    shared_memory_fd = shm_open(message.shared_memory_name, O_RDWR, 0600);
    if (shared_memory_fd < 0)
        return (-1);
    mapping_pointer = mmap(ft_nullptr, message.remote_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    ::close(shared_memory_fd);
    if (mapping_pointer == MAP_FAILED)
        return (-1);
    mapping->mapping_address = reinterpret_cast<unsigned char *>(mapping_pointer);
    mapping->mapping_length = static_cast<ft_size_t>(message.remote_memory_size);
    mapping->platform_handle = ft_nullptr;
    return (0);
}

int cmp_cross_process_close_mapping(cmp_cross_process_mapping *mapping)
{
    if (mapping->mapping_address == ft_nullptr)
        return (0);
    if (munmap(mapping->mapping_address, mapping->mapping_length) != 0)
        return (-1);
    mapping->mapping_address = ft_nullptr;
    mapping->mapping_length = 0;
    mapping->platform_handle = ft_nullptr;
    return (0);
}

int cmp_cross_process_lock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    ft_size_t mutex_offset;
    pthread_mutex_t *shared_mutex;
    int attempt_count;

    mutex_offset = compute_offset(message.shared_mutex_address, message.stack_base_address);
    if (mutex_offset + static_cast<ft_size_t>(sizeof(pthread_mutex_t)) > mapping->mapping_length)
    {
        errno = EINVAL;
        return (-1);
    }
    shared_mutex = reinterpret_cast<pthread_mutex_t *>(mapping->mapping_address + mutex_offset);
    attempt_count = 0;
    while (attempt_count < 5)
    {
        int lock_result;

        lock_result = pthread_mutex_trylock(shared_mutex);
        if (lock_result == 0)
        {
            mutex_state->platform_mutex = shared_mutex;
            return (0);
        }
        if (lock_result != EBUSY)
        {
            errno = lock_result;
            return (-1);
        }
        attempt_count += 1;
        if (attempt_count >= 5)
            break;
        usleep(50000);
    }
    errno = ETIMEDOUT;
    return (-1);
}

int cmp_cross_process_unlock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state)
{
    pthread_mutex_t *shared_mutex;

    (void)message;
    (void)mapping;
    shared_mutex = reinterpret_cast<pthread_mutex_t *>(mutex_state->platform_mutex);
    if (shared_mutex == ft_nullptr)
        return (0);
    int unlock_error = pt_pthread_mutex_unlock_with_error(shared_mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
        return (-1);
    mutex_state->platform_mutex = ft_nullptr;
    return (0);
}

#endif
