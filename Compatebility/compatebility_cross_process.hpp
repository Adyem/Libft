#ifndef COMPATEBILITY_CROSS_PROCESS_HPP
#define COMPATEBILITY_CROSS_PROCESS_HPP

#include "../CrossProcess/cross_process.hpp"

struct cmp_cross_process_mapping
{
    unsigned char *mapping_address;
    ft_size_t mapping_length;
    void *platform_handle;
    void *mutex_address;
};

struct cmp_cross_process_mutex_state
{
    void *platform_mutex;
};

int cmp_cross_process_send_descriptor(int socket_fd, const cross_process_message &message);
int cmp_cross_process_receive_descriptor(int socket_fd, cross_process_message &message);
int cmp_cross_process_open_mapping(const cross_process_message &message, cmp_cross_process_mapping *mapping);
int cmp_cross_process_close_mapping(cmp_cross_process_mapping *mapping);
int cmp_cross_process_lock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state);
int cmp_cross_process_unlock_mutex(const cross_process_message &message, cmp_cross_process_mapping *mapping, cmp_cross_process_mutex_state *mutex_state);

#endif
