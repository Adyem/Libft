#include "networking.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include <cerrno>

int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds)
{
    int kqueue_descriptor;
    struct kevent change_event;
    struct kevent *event_list;
    int index;
    int maximum_events;
    int ready_descriptors;
    int ready_index;
    int descriptor;
    int search_index;
    timespec timeout;

    kqueue_descriptor = kqueue();
    if (kqueue_descriptor == -1)
    {
        (void)(ft_map_system_error(errno));
        return (-1);
    }
    index = 0;
    while (read_file_descriptors && index < read_count)
    {
        EV_SET(&change_event, read_file_descriptors[index], EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kqueue_descriptor, &change_event, 1, NULL, 0, NULL) == -1)
        {
            int last_error;

            last_error = errno;
            close(kqueue_descriptor);
            (void)(ft_map_system_error(last_error));
            return (-1);
        }
        index++;
    }
    index = 0;
    while (write_file_descriptors && index < write_count)
    {
        EV_SET(&change_event, write_file_descriptors[index], EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(kqueue_descriptor, &change_event, 1, NULL, 0, NULL) == -1)
        {
            int last_error;

            last_error = errno;
            close(kqueue_descriptor);
            (void)(ft_map_system_error(last_error));
            return (-1);
        }
        index++;
    }
    maximum_events = read_count + write_count;
    if (maximum_events == 0)
    {
        close(kqueue_descriptor);
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    event_list = static_cast<struct kevent *>(cma_malloc(sizeof(struct kevent) * maximum_events));
    if (!event_list)
    {
        close(kqueue_descriptor);
        (void)(FT_ERR_NO_MEMORY);
        return (-1);
    }
    timeout.tv_sec = timeout_milliseconds / 1000;
    timeout.tv_nsec = (timeout_milliseconds % 1000) * 1000000;
    ready_descriptors = kevent(kqueue_descriptor, NULL, 0, event_list, maximum_events, &timeout);
    if (ready_descriptors <= 0)
    {
        int wait_error;

        wait_error = errno;
        cma_free(event_list);
        close(kqueue_descriptor);
        if (ready_descriptors == 0)
            (void)(FT_ERR_SUCCESS);
        else
            (void)(ft_map_system_error(wait_error));
        return (ready_descriptors);
    }
    ready_index = 0;
    while (ready_index < ready_descriptors)
    {
        descriptor = static_cast<int>(event_list[ready_index].ident);
        search_index = 0;
        while (read_file_descriptors && search_index < read_count)
        {
            if (read_file_descriptors[search_index] == descriptor)
                break ;
            search_index++;
        }
        if (read_file_descriptors && search_index < read_count)
            read_file_descriptors[search_index] = descriptor;
        search_index = 0;
        while (write_file_descriptors && search_index < write_count)
        {
            if (write_file_descriptors[search_index] == descriptor)
                break ;
            search_index++;
        }
        if (write_file_descriptors && search_index < write_count)
            write_file_descriptors[search_index] = descriptor;
        ready_index++;
    }
    cma_free(event_list);
    close(kqueue_descriptor);
    (void)(FT_ERR_SUCCESS);
    return (ready_descriptors);
}
