#include "networking.hpp"
#include "../Errno/errno.hpp"
#ifdef _WIN32
# include <winsock2.h>
#else
# include <sys/select.h>
# include <unistd.h>
# include <cerrno>
#endif

int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds)
{
#ifdef _WIN32
    fd_set read_set;
    fd_set write_set;
    int index;
    int max_descriptor;
    int ready_descriptors;
    int total_ready;
    timeval timeout;
    timeval *timeout_pointer;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    index = 0;
    max_descriptor = -1;
    timeout_pointer = NULL;
    while (read_file_descriptors && index < read_count)
    {
        if (read_file_descriptors[index] >= 0)
        {
            FD_SET(static_cast<SOCKET>(read_file_descriptors[index]), &read_set);
            if (read_file_descriptors[index] > max_descriptor)
                max_descriptor = read_file_descriptors[index];
        }
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (write_file_descriptors[index] >= 0)
        {
            FD_SET(static_cast<SOCKET>(write_file_descriptors[index]), &write_set);
            if (write_file_descriptors[index] > max_descriptor)
                max_descriptor = write_file_descriptors[index];
        }
        index++;
    }
    if (timeout_milliseconds >= 0)
    {
        timeout.tv_sec = timeout_milliseconds / 1000;
        timeout.tv_usec = (timeout_milliseconds % 1000) * 1000;
        timeout_pointer = &timeout;
    }
    ready_descriptors = select(max_descriptor + 1, &read_set, &write_set, NULL, timeout_pointer);
    if (ready_descriptors <= 0)
    {
        int select_error;

        select_error = WSAGetLastError();
        if (ready_descriptors == 0)
            ft_errno = ER_SUCCESS;
        else
            ft_errno = select_error + ERRNO_OFFSET;
        return (ready_descriptors);
    }
    index = 0;
    total_ready = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (read_file_descriptors[index] < 0 ||
            !FD_ISSET(static_cast<SOCKET>(read_file_descriptors[index]), &read_set))
            read_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (write_file_descriptors[index] < 0 ||
            !FD_ISSET(static_cast<SOCKET>(write_file_descriptors[index]), &write_set))
            write_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (total_ready);
#else
    fd_set read_set;
    fd_set write_set;
    int index;
    int max_descriptor;
    int ready_descriptors;
    int total_ready;
    timeval timeout;
    timeval *timeout_pointer;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    index = 0;
    max_descriptor = -1;
    timeout_pointer = NULL;
    while (read_file_descriptors && index < read_count)
    {
        if (read_file_descriptors[index] >= 0)
        {
            FD_SET(read_file_descriptors[index], &read_set);
            if (read_file_descriptors[index] > max_descriptor)
                max_descriptor = read_file_descriptors[index];
        }
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (write_file_descriptors[index] >= 0)
        {
            FD_SET(write_file_descriptors[index], &write_set);
            if (write_file_descriptors[index] > max_descriptor)
                max_descriptor = write_file_descriptors[index];
        }
        index++;
    }
    if (timeout_milliseconds >= 0)
    {
        timeout.tv_sec = timeout_milliseconds / 1000;
        timeout.tv_usec = (timeout_milliseconds % 1000) * 1000;
        timeout_pointer = &timeout;
    }
    ready_descriptors = select(max_descriptor + 1, &read_set, &write_set, NULL, timeout_pointer);
    if (ready_descriptors <= 0)
    {
        int select_error;

        select_error = errno;
        if (ready_descriptors == 0)
            ft_errno = ER_SUCCESS;
        else
            ft_errno = select_error + ERRNO_OFFSET;
        return (ready_descriptors);
    }
    index = 0;
    total_ready = 0;
    while (read_file_descriptors && index < read_count)
    {
        if (read_file_descriptors[index] < 0 ||
            !FD_ISSET(read_file_descriptors[index], &read_set))
            read_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        if (write_file_descriptors[index] < 0 ||
            !FD_ISSET(write_file_descriptors[index], &write_set))
            write_file_descriptors[index] = -1;
        else
            total_ready++;
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (total_ready);
#endif
}
