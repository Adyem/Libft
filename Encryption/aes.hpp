#ifndef AES_HPP
#define AES_HPP

#include <stdint.h>

void    aes_encrypt(uint8_t *block, const uint8_t *key);
void    aes_decrypt(uint8_t *block, const uint8_t *key);

void    aes_encrypt_software(uint8_t *block, const uint8_t *key);
void    aes_decrypt_software(uint8_t *block, const uint8_t *key);

#endif
