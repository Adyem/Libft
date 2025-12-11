#include "udp_socket.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int udp_event_loop_wait_internal(event_loop *loop, udp_socket &socket,
                                        bool is_write, int timeout_milliseconds)
{
    int socket_fd;
    int descriptor_index;
    int poll_result;
    bool descriptor_ready;
    int saved_error_code;

    if (loop == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    socket_fd = socket.get_fd();
    if (socket_fd < 0)
    {
        ft_errno = FT_ERR_CONFIGURATION;
        return (-1);
    }
    if (event_loop_add_socket(loop, socket_fd, is_write) != 0)
        return (-1);
    if (is_write)
        descriptor_index = loop->write_count - 1;
    else
        descriptor_index = loop->read_count - 1;
    poll_result = event_loop_run(loop, timeout_milliseconds);
    descriptor_ready = false;
    if (poll_result > 0)
    {
        if (is_write)
        {
            if (loop->write_file_descriptors && descriptor_index >= 0
                && descriptor_index < loop->write_count)
            {
                if (loop->write_file_descriptors[descriptor_index] == socket_fd)
                    descriptor_ready = true;
            }
        }
        else
        {
            if (loop->read_file_descriptors && descriptor_index >= 0
                && descriptor_index < loop->read_count)
            {
                if (loop->read_file_descriptors[descriptor_index] == socket_fd)
                    descriptor_ready = true;
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
    saved_error_code = ft_errno;
    event_loop_remove_socket(loop, socket_fd, is_write);
    if (poll_result < 0)
    {
        ft_errno = saved_error_code;
        return (-1);
    }
    if (poll_result == 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (!descriptor_ready)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

int udp_event_loop_wait_read(event_loop *loop, udp_socket &socket, int timeout_milliseconds)
{
    return (udp_event_loop_wait_internal(loop, socket, false, timeout_milliseconds));
}

int udp_event_loop_wait_write(event_loop *loop, udp_socket &socket, int timeout_milliseconds)
{
    return (udp_event_loop_wait_internal(loop, socket, true, timeout_milliseconds));
}

ssize_t udp_event_loop_receive(event_loop *loop, udp_socket &socket, void *buffer, size_t size,
                               int flags, struct sockaddr *source_address,
                               socklen_t *address_length, int timeout_milliseconds)
{
    int wait_result;

    if (size > 0 && buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    wait_result = udp_event_loop_wait_internal(loop, socket, false, timeout_milliseconds);
    if (wait_result < 0)
        return (-1);
    if (wait_result == 0)
        return (0);
    return (socket.receive_from(buffer, size, flags, source_address, address_length));
}

ssize_t udp_event_loop_send(event_loop *loop, udp_socket &socket, const void *data, size_t size,
                            int flags, const struct sockaddr *destination_address,
                            socklen_t address_length, int timeout_milliseconds)
{
    int wait_result;

    if (size > 0 && data == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    wait_result = udp_event_loop_wait_internal(loop, socket, true, timeout_milliseconds);
    if (wait_result < 0)
        return (-1);
    if (wait_result == 0)
        return (0);
    return (socket.send_to(data, size, flags, destination_address, address_length));
}

