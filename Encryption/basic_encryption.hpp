#ifndef BASIC_ENCRYPTION_HPP
#define BASIC_ENCRYPTION_HPP

#include <cstddef>
#include <sys/types.h>
#include "aes.hpp"
#include "rsa.hpp"
#include "encryption_sha256.hpp"
#include "encryption_hmac_sha256.hpp"

typedef int (*t_be_open_function)(const char *path_name, int flags, mode_t mode);
typedef ssize_t (*t_be_write_function)(int file_descriptor, const void *buffer, size_t count);

int            be_saveGame(const char *filename, const char *data, const char *key);
char        **be_DecryptData(char **data, const char *key);
const char    *be_getEncryptionKey();
void            be_set_save_game_hooks(t_be_open_function open_function,
                    t_be_write_function write_function);



#endif
