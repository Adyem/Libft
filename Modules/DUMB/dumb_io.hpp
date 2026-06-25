#pragma once

#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

int32_t ft_read_file(const char *path, char **out_buffer, ft_size_t *out_size);
int32_t ft_write_file(const char *path, const char *buffer, ft_size_t size);
