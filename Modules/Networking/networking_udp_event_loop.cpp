#include "udp_socket.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static int32_t udp_event_loop_wait_internal(event_loop *loop, udp_socket &socket,
                                        ft_bool is_write, int32_t timeout_milliseconds)
{
    int32_t socket_fd;
    int32_t descriptor_index;
    int32_t poll_result;
    ft_bool descriptor_ready;

    if (loop == ft_nullptr)
        return (-1);
    socket_fd = socket.get_fd();
    if (socket_fd < 0)
        return (-1);
    if (event_loop_add_socket(loop, socket_fd, is_write) != 0)
        return (-1);
    if (is_write)
        descriptor_index = loop->write_count - 1;
    else
        descriptor_index = loop->read_count - 1;
    poll_result = event_loop_run(loop, timeout_milliseconds);
    descriptor_ready = FT_FALSE;
    if (poll_result > 0)
    {
        if (is_write)
        {
            if (loop->write_file_descriptors && descriptor_index >= 0
                && descriptor_index < loop->write_count)
            {
                if (loop->write_file_descriptors[descriptor_index] == socket_fd)
                    descriptor_ready = FT_TRUE;
            }
        }
        else
        {
            if (loop->read_file_descriptors && descriptor_index >= 0
                && descriptor_index < loop->read_count)
            {
                if (loop->read_file_descriptors[descriptor_index] == socket_fd)
                    descriptor_ready = FT_TRUE;
            }
        }
    }
    if (is_write)
    {
        if (loop->write_file_descriptors && descriptor_index >= 0
            && descriptor_index < loop->write_count)
            loop->write_file_descriptors[descriptor_index] = socket_fd;
    }
    else
    {
        if (loop->read_file_descriptors && descriptor_index >= 0
            && descriptor_index < loop->read_count)
            loop->read_file_descriptors[descriptor_index] = socket_fd;
    }
    event_loop_remove_socket(loop, socket_fd, is_write);
    if (poll_result < 0)
        return (-1);
    if (poll_result == 0)
        return (0);
    if (!descriptor_ready)
        return (0);
    return (1);
}

int32_t udp_event_loop_wait_read(event_loop *loop, udp_socket &socket, int32_t timeout_milliseconds)
{
    return (udp_event_loop_wait_internal(loop, socket, FT_FALSE, timeout_milliseconds));
}

int32_t udp_event_loop_wait_write(event_loop *loop, udp_socket &socket, int32_t timeout_milliseconds)
{
    return (udp_event_loop_wait_internal(loop, socket, FT_TRUE, timeout_milliseconds));
}

ssize_t udp_event_loop_receive(event_loop *loop, udp_socket &socket, void *buffer, ft_size_t size,
                               int32_t flags, struct sockaddr *source_address,
                               socklen_t *address_length, int32_t timeout_milliseconds)
{
    int32_t wait_result;

    if (size > 0 && buffer == ft_nullptr)
        return (-1);
    wait_result = udp_event_loop_wait_internal(loop, socket, FT_FALSE, timeout_milliseconds);
    if (wait_result < 0)
        return (-1);
    if (wait_result == 0)
        return (0);
    return (socket.receive_from(buffer, size, flags, source_address, address_length));
}

ssize_t udp_event_loop_send(event_loop *loop, udp_socket &socket, const void *data, ft_size_t size,
                            int32_t flags, const struct sockaddr *destination_address,
                            socklen_t address_length, int32_t timeout_milliseconds)
{
    int32_t wait_result;

    if (size > 0 && data == ft_nullptr)
        return (-1);
    wait_result = udp_event_loop_wait_internal(loop, socket, FT_TRUE, timeout_milliseconds);
    if (wait_result < 0)
        return (-1);
    if (wait_result == 0)
        return (0);
    return (socket.send_to(data, size, flags, destination_address, address_length));
}
