#ifndef ENCRYPTION_SECURE_WIPE_HPP
# define ENCRYPTION_SECURE_WIPE_HPP

#include <cstddef>

int     encryption_secure_wipe(void *buffer, std::size_t buffer_size);
int     encryption_secure_wipe_string(char *string_buffer);

#endif
