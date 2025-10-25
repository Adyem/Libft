#ifndef ENCRYPTION_HARDWARE_ACCELERATION_HPP
# define ENCRYPTION_HARDWARE_ACCELERATION_HPP

#include <stdint.h>
#include <stddef.h>

typedef void (*t_encryption_aes_block_function)(uint8_t *block, const uint8_t *key);

struct s_encryption_hardware_hooks
{
    t_encryption_aes_block_function aes_encrypt;
    t_encryption_aes_block_function aes_decrypt;
};

int     encryption_register_hardware_hooks(const s_encryption_hardware_hooks &hooks);
void    encryption_clear_hardware_hooks(void);
void    encryption_get_hardware_hooks(s_encryption_hardware_hooks &out_hooks);
bool    encryption_try_hardware_aes_encrypt(uint8_t *block, const uint8_t *key);
bool    encryption_try_hardware_aes_decrypt(uint8_t *block, const uint8_t *key);

#endif
