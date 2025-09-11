#ifndef RSA_HPP
#define RSA_HPP

#include <stdint.h>

int        rsa_generate_key_pair(uint64_t *public_key, uint64_t *private_key, uint64_t *modulus, int bit_size);
uint64_t    rsa_encrypt(uint64_t message, uint64_t public_key, uint64_t modulus);
uint64_t    rsa_decrypt(uint64_t cipher, uint64_t private_key, uint64_t modulus);

/*
Usage:
    uint64_t public_key;
    uint64_t private_key;
    uint64_t modulus;
    rsa_generate_key_pair(&public_key, &private_key, &modulus, 32);
    uint64_t encrypted = rsa_encrypt(42, public_key, modulus);
    uint64_t decrypted = rsa_decrypt(encrypted, private_key, modulus);
*/

#endif
