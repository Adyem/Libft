#ifndef CROSS_PROCESS_HPP
#define CROSS_PROCESS_HPP

#include <cstdint>

#include "../CPP_class/class_string.hpp"
#include "../Libft/libft.hpp"

struct cross_process_message
{
    uint64_t stack_base_address;
    uint64_t remote_memory_address;
    uint64_t remote_memory_size;
    uint64_t shared_mutex_address;
    uint64_t error_memory_address;
    char shared_memory_name[256];
};

struct cross_process_read_result
{
    ft_string shared_memory_name;
    ft_string payload;
};

int cp_send_descriptor(int socket_fd, const cross_process_message &message);
int cp_receive_descriptor(int socket_fd, cross_process_message &message);
int cp_receive_memory(int socket_fd, cross_process_read_result &result);
int cp_write_memory(const cross_process_message &message, const unsigned char *payload, ft_size_t payload_length, int error_code);

#endif
