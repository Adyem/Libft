#pragma once

#include <stddef.h>

enum ft_io_error
{
    ft_io_ok = 0,
    ft_io_error_invalid_argument = 1,
    ft_io_error_file_not_found = 2,
    ft_io_error_permission_denied = 3,
    ft_io_error_read_failed = 4,
    ft_io_error_write_failed = 5
};

int ft_read_file(const char *path, char **out_buffer, size_t *out_size);
int ft_write_file(const char *path, const char *buffer, size_t size);
